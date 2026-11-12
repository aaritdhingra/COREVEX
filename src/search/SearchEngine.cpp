// ============================================================================
// SECTION 11 (impl) — SEARCH ENGINE
// ============================================================================
#include "SearchEngine.h"

#include <QSettings>
#include <QUrlQuery>

namespace corevex {

QString searchEngineId() {
    QSettings s(QStringLiteral("COREVEX"), QStringLiteral("Browser"));
    // v4.5: custom search page ke liye Brave pehle — uske result links
    // direct hote hain (Bing ke ck redirect nahi), isliye display URL
    // kabhi www.bing.com nahi dikhega. Flag ke baad user jo chune, wahi rahega.
    if (!s.value(QStringLiteral("search/migratedToBraveV45"), false).toBool()) {
        s.setValue(QStringLiteral("search/engine"), QStringLiteral("brave"));
        s.setValue(QStringLiteral("search/migratedToBraveV45"), true);
        return QStringLiteral("brave");
    }
    const QString id = s.value(QStringLiteral("search/engine"), QStringLiteral("bing")).toString();
    if (id == QStringLiteral("google") || id == QStringLiteral("bing") || id == QStringLiteral("brave") ||
        id == QStringLiteral("mojeek"))
        return id;
    return QStringLiteral("bing");
}

void setSearchEngineId(const QString& id) {
    QSettings s(QStringLiteral("COREVEX"), QStringLiteral("Browser"));
    s.setValue(QStringLiteral("search/engine"), id);
}

QString searchEngineName(const QString& id) {
    if (id == QStringLiteral("google")) return QStringLiteral("Google");
    if (id == QStringLiteral("bing")) return QStringLiteral("Bing");
    if (id == QStringLiteral("brave")) return QStringLiteral("Brave Search");
    if (id == QStringLiteral("mojeek")) return QStringLiteral("Mojeek");
    return QStringLiteral("Bing");
}

QUrl searchUrlForEngine(const QString& query, const QString& id) {
    // v4.2: COREVEX Search fetch (Bing parser) toota hua tha — Bing ke
    // ck/a redirect links se har result ka URL www.bing.com dikhta tha
    // aur snippet khaali rehta tha, isliye cards trash lagte the.
    // Ab search seedha engine ke asli page par jaata hai: Google par
    // Google ke asli results, asli URL, asli snippet. COREVEX ka kaam
    // sirf theme injection hai (ThemeInjector), jo Google ke page ko
    // dark + orange COREVEX theme me dikhata hai.
    QString base;
    if (id == QStringLiteral("bing")) base = QStringLiteral("https://www.bing.com/search");
    else if (id == QStringLiteral("brave")) base = QStringLiteral("https://search.brave.com/search");
    else if (id == QStringLiteral("mojeek")) base = QStringLiteral("https://www.mojeek.com/search");
    else base = QStringLiteral("https://www.google.com/search");
    QUrl url(base);
    QUrlQuery q;
    q.addQueryItem(QStringLiteral("q"), query.trimmed());
    url.setQuery(q);
    return url;
}

QUrl searchUrl(const QString& query) {
    return searchUrlForEngine(query, searchEngineId());
}

QString searchEngineBaseUrl(const QString& id) {
    // v4.5: new-tab form corevex://search par jaata hai, taaki COREVEX ka
    // apna search page (last image theme) khule, engine ka page nahi.
    // Google direct chuna ho to form bhi Google par jaata hai.
    Q_UNUSED(id);
    if (id == QStringLiteral("google")) return QStringLiteral("https://www.google.com/search");
    return QStringLiteral("corevex://search");
}

}  // namespace corevex
