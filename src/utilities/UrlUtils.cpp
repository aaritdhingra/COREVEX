// ============================================================================
// SECTION 2 (impl) — ADDRESS-BAR URL RESOLUTION (FIXED: single-word bug)
// ----------------------------------------------------------------------------
// Bug jo screenshot me dikha (2026-10-30):
//   User ne address bar me "youtube" likha, app ne usko
//   http://youtube/ bana diya, DNS fail hua (ERR_NAME_NOT_RESOLVED).
//   Wajah: QUrl::fromUserInput("youtube") ek single word ko bhi host
//   maan leta hai, aur hamara purana code usko safe URL samajh leta tha.
// Fix — sirf tab URL maano jab:
//   * input me scheme ho (https://...), ya
//   * host me dot ho (youtube.com, www.google.com), ya
//   * host localhost / IP ho, ya path me '/' ho (site.com/page)
//   Baaki sab — single word ("youtube", "aaj ka weather"), spaces,
//   ya bina dot wala text — SEARCH query hai, URL nahi.
//   Aur: http:// wale normal sites ko https:// par upgrade karo
//   (localhost chhodkar), taaki http redirect ke chakkar me na fase.
// Search ab Google par hardcode nahi hai (Section 11: default DuckDuckGo).
// ============================================================================
#include "UrlUtils.h"

#include "../search/SearchEngine.h"
#include <QUrlQuery>

namespace corevex {

QUrl googleSearchUrl(const QString& query) {
    return searchUrl(query);
}

bool isSafeWebUrl(const QUrl& url) {
    if (!url.isValid() || url.host().isEmpty()) return false;
    const QString scheme = url.scheme().toLower();
    return scheme == QStringLiteral("http") || scheme == QStringLiteral("https");
}

// Ek word se famous site ka poora domain — "youtube" likhne par bhi
// seedha site khulni chahiye, search nahi (ye user expectation hai).
static QString famousSiteDomain(const QString& word) {
    static const struct { const char* word; const char* domain; } kMap[] = {
        {"youtube", "www.youtube.com"}, {"google", "www.google.com"},
        {"github", "github.com"}, {"leetcode", "leetcode.com"},
        {"linkedin", "www.linkedin.com"}, {"chatgpt", "chatgpt.com"},
        {"instagram", "www.instagram.com"}, {"facebook", "www.facebook.com"},
        {"twitter", "x.com"}, {"x", "x.com"}, {"reddit", "www.reddit.com"},
        {"wikipedia", "www.wikipedia.org"}, {"amazon", "www.amazon.in"},
        {"flipkart", "www.flipkart.com"},
    };
    for (const auto& e : kMap)
        if (word == QLatin1String(e.word)) return QString::fromLatin1(e.domain);
    return QString();
}

QUrl resolveAddressInput(const QString& input) {
    const QString text = input.trimmed();
    if (text.isEmpty()) return QUrl(QStringLiteral("about:blank"));
    const QString lower = text.toLower();
    for (const char* scheme : {"javascript:", "data:", "file:", "vbscript:"}) {
        if (lower.startsWith(QLatin1String(scheme))) return searchUrl(text);
    }
    // Scheme ke saath poora URL — seedha kholo, https par upgrade karo.
    if (lower.startsWith(QStringLiteral("http://")) || lower.startsWith(QStringLiteral("https://"))) {
        QUrl url(text);
        if (isSafeWebUrl(url)) {
            if (url.scheme() == QStringLiteral("http") && url.host() != QStringLiteral("localhost"))
                url.setScheme(QStringLiteral("https"));
            return url;
        }
        return searchUrl(text);
    }
    // Spaces hain to ye query hai, URL kabhi nahi.
    if (text.contains(' ')) return searchUrl(text);
    // Single famous word — seedha site (youtube -> youtube.com).
    if (const QString domain = famousSiteDomain(lower); !domain.isEmpty()) {
        QUrl url(QStringLiteral("https://") + domain);
        return url;
    }
    // Dot / slash wala text hi URL ho sakta hai (youtube.com, a.com/b).
    // Bina dot wala single word ("youtube", "weather") SEARCH hai.
    const bool looksLikeUrl = text.contains('.') || text.contains('/');
    if (!looksLikeUrl) return searchUrl(text);
    QUrl url = QUrl::fromUserInput(text);
    if (isSafeWebUrl(url)) {
        // www. ya domain ke saath http ko https par upgrade karo.
        if (url.scheme() == QStringLiteral("http") && url.host() != QStringLiteral("localhost"))
            url.setScheme(QStringLiteral("https"));
        return url;
    }
    return searchUrl(text);
}

}  // namespace corevex
