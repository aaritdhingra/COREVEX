// ============================================================================
// SECTION 13 (impl) — COREVEX SEARCH
// ============================================================================
#include "CustomSearch.h"
#include "SearchEngine.h"
#include "../utilities/Assets.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>

namespace corevex {

static QString htmlEscape(const QString& s) {
    QString r = s;
    r.replace('&', QStringLiteral("&amp;"));
    r.replace('<', QStringLiteral("&lt;"));
    r.replace('>', QStringLiteral("&gt;"));
    r.replace('"', QStringLiteral("&quot;"));
    r.replace('\'', QStringLiteral("&#39;"));
    return r;
}

QString CustomSearch::stripTags(const QString& html) {
    QString r = html;
    r.replace(QRegularExpression(QStringLiteral("<[^>]+>")), QStringLiteral(" "));
    r.replace(QStringLiteral("&amp;"), QStringLiteral("&"));
    r.replace(QStringLiteral("&quot;"), QStringLiteral("\""));
    r.replace(QStringLiteral("&#39;"), QStringLiteral("'"));
    r.replace(QStringLiteral("&lt;"), QStringLiteral("<"));
    r.replace(QStringLiteral("&gt;"), QStringLiteral(">"));
    r.replace(QStringLiteral("&nbsp;"), QStringLiteral(" "));
    r.replace(QRegularExpression(QStringLiteral("\\s+")), QStringLiteral(" "));
    return r.trimmed();
}

QString CustomSearch::decodeDdgLink(const QString& href) {
    QString h = href.trimmed();
    if (h.startsWith(QStringLiteral("//"))) h = QStringLiteral("https:") + h;
    // DuckDuckGo redirect: //duckduckgo.com/l/?uddg=<real url, encoded>
    const int idx = h.indexOf(QStringLiteral("uddg="));
    if (idx >= 0) {
        QString enc = h.mid(idx + 5);
        const int amp = enc.indexOf('&');
        if (amp >= 0) enc = enc.left(amp);
        // Qt 6.7.3 SACH: fromPercentEncoding(const QByteArray&) -> QString.
        // Isliye na to fromUtf8() chahiye (wo QString nahi leta), na hi seedha QString arg chahiye.
        // Sahi: enc.toUtf8() (QByteArray) do, wapas QString milega, seedha use karo.
        const QString decoded = QUrl::fromPercentEncoding(enc.toUtf8());
        if (!decoded.isEmpty()) return decoded;
    }
    return h;
}

// --- HTML pages (COREVEX custom theme, koi engine theme nahi) --------------
QString searchResultsHtml(const QString& query, const QList<SearchResult>& results,
                          const QString& sourceName) {
    Q_UNUSED(sourceName);
    // v4.5 — last image jaisa search page, pixel-by-pixel theme:
    // header (logo + search box + Tools + theme toggle), tabs (All / AI
    // Overview / Images / Videos / News / Discussions / Shopping),
    // main column (AI Overview card + results), right column
    // (Research tools / Related searches / Sources).
    // KOI comment/meta line nahi: na pill, na "via Bing" line, na footer
    // note, na orange highlight wali comment lines. Sirf results.
    const QString bg = assetDataUri(QStringLiteral("backgrounds/bg_small.jpg"));
    const QString qEsc = htmlEscape(query);
    const QString qUrl = QString::fromUtf8(QUrl::toPercentEncoding(query));
    const QString bingUrl = QStringLiteral("https://www.bing.com/search?q=") + qUrl;
    QString html = QStringLiteral(R"HTML(
<html><head><meta charset='utf-8'><title>%1 - COREVEX</title><style>
body{margin:0;background:#0B0C0F;color:#F4F4F5;font-family:'Segoe UI',sans-serif;}
body.light{background:#FFFFFF;color:#111111;}
.bg{position:fixed;inset:0;background-size:cover;background-position:center;opacity:.35;pointer-events:none;z-index:0;}
.scrim{position:fixed;inset:0;background:rgba(0,0,0,.55);pointer-events:none;z-index:0;}
body.light .bg,body.light .scrim{display:none;}
.wrap{position:relative;z-index:1;max-width:1240px;margin:0 auto;padding:0 20px 40px;}
.header{display:flex;align-items:center;gap:16px;padding:16px 0;border-bottom:1px solid #2A2D35;flex-wrap:wrap;}
.logo{font-size:24px;font-weight:800;letter-spacing:-.5px;white-space:nowrap;}
.logo b{color:#FF5A1F;}
.hform{flex:1;display:flex;align-items:center;background:#14161B;border:1px solid #FF5A1F;border-radius:28px;padding:4px 4px 4px 18px;min-width:280px;}
body.light .hform{background:#fff;border-color:#FF5A1F;}
.hform input{flex:1;background:transparent;border:none;color:#fff;font-size:15px;outline:none;min-width:0;}
body.light .hform input{color:#111;}
.hbtn{background:#FF5A1F;border:none;border-radius:50%;width:40px;height:40px;color:#fff;font-size:18px;cursor:pointer;flex-shrink:0;}
.tools{position:relative;}
.tools summary{list-style:none;cursor:pointer;background:#14161B;border:1px solid #2A2D35;border-radius:20px;padding:10px 16px;font-size:14px;}
.tools[open] .tmenu{position:absolute;right:0;top:44px;background:#1B1E24;border:1px solid #2A2D35;border-radius:12px;padding:8px;min-width:220px;z-index:5;}
.tmenu a{display:block;color:#F4F4F5;text-decoration:none;padding:10px 12px;border-radius:8px;font-size:14px;}
.tmenu a:hover{background:#2A2D35;}
.themebtn{background:#14161B;border:1px solid #2A2D35;border-radius:50%;width:40px;height:40px;color:#F4F4F5;font-size:18px;cursor:pointer;}
.tabs{display:flex;gap:24px;padding:14px 0;border-bottom:1px solid #2A2D35;flex-wrap:wrap;}
.tabs a{color:#A1A1AA;text-decoration:none;font-size:14px;padding-bottom:8px;}
.tabs a.active{color:#FF5A1F;border-bottom:2px solid #FF5A1F;font-weight:700;}
.layout{display:flex;gap:20px;margin-top:20px;align-items:flex-start;}
.main{flex:1;min-width:0;}
.side{width:320px;flex-shrink:0;}
@media(max-width:900px){.layout{flex-direction:column;}.side{width:100%;}}
.card{background:rgba(20,22,27,.94);border:1px solid #2A2D35;border-radius:14px;padding:20px;margin-bottom:16px;}
body.light .card{background:#fff;border-color:#E5E7EB;}
.ai{border-left:2px solid #FF5A1F;}
.aihead{display:flex;align-items:center;justify-content:space-between;gap:12px;flex-wrap:wrap;}
.aihead h2{margin:0;font-size:18px;}
.ask{border:1px solid #FF5A1F;color:#FF5A1F;background:transparent;border-radius:20px;padding:8px 16px;text-decoration:none;font-size:13px;font-weight:700;white-space:nowrap;}
.aibody{color:#D4D4D8;line-height:1.7;font-size:15px;margin-top:12px;}
body.light .aibody{color:#333;}
.aibody sup a{color:#FF5A1F;text-decoration:none;}
.chips{display:flex;gap:10px;flex-wrap:wrap;margin-top:14px;}
.chip{display:flex;align-items:center;gap:8px;background:#1B1E24;border:1px solid #2A2D35;border-radius:20px;padding:6px 12px;text-decoration:none;color:#F4F4F5;font-size:13px;}
body.light .chip{background:#F3F4F6;border-color:#E5E7EB;color:#111;}
.chip img{width:20px;height:20px;border-radius:4px;}
.result{display:block;text-decoration:none;color:inherit;background:rgba(20,22,27,.94);border:1px solid #2A2D35;border-radius:14px;padding:18px 20px;margin-bottom:14px;}
body.light .result{background:#fff;border-color:#E5E7EB;}
.result:hover{border-color:#FF5A1F;}
.rpath{display:flex;align-items:center;gap:8px;color:#A1A1AA;font-size:13px;word-break:break-all;}
.rpath img{width:20px;height:20px;border-radius:4px;flex-shrink:0;}
.rtitle{color:#5B9DFF;font-size:19px;font-weight:600;line-height:1.4;margin-top:8px;}
.result:hover .rtitle{text-decoration:underline;}
.rsnip{color:#A1A1AA;font-size:14px;line-height:1.6;margin-top:8px;}
body.light .rsnip{color:#555;}
.side h3{margin:0 0 14px;font-size:16px;}
.toolrow{display:flex;justify-content:space-between;align-items:center;text-decoration:none;color:#F4F4F5;background:#1B1E24;border:1px solid #2A2D35;border-radius:10px;padding:12px 14px;margin-bottom:10px;font-size:14px;}
body.light .toolrow{background:#F3F4F6;color:#111;border-color:#E5E7EB;}
.toolrow:hover{border-color:#FF5A1F;}
.rel a{display:flex;gap:10px;align-items:center;text-decoration:none;color:#F4F4F5;padding:10px 0;border-bottom:1px solid #2A2D35;font-size:14px;}
body.light .rel a{color:#111;}
.srcrow{display:flex;gap:12px;align-items:center;text-decoration:none;color:inherit;padding:10px 0;border-bottom:1px solid #2A2D35;}
.srcnum{background:#FF5A1F;color:#fff;border-radius:50%;width:26px;height:26px;display:flex;align-items:center;justify-content:center;font-size:13px;font-weight:700;flex-shrink:0;}
.srcdom{font-weight:600;font-size:14px;}
.srcurl{color:#A1A1AA;font-size:12px;margin-left:auto;word-break:break-all;text-align:right;}
.empty{color:#A1A1AA;line-height:1.7;padding:20px;}
</style></head><body>
)HTML").arg(qEsc);
    if (!bg.isEmpty())
        html += QStringLiteral("<div class='bg' style=\"background-image:url('%1')\"></div><div class='scrim'></div>").arg(bg);
    html += QStringLiteral(R"HTML(<div class='wrap'>
<div class='header'>
<div class='logo'><b>C</b>OREVEX</div>
<form class='hform' action='corevex://search' method='get'><input name='q' value='%1' /><button class='hbtn' type='submit'>&rarr;</button></form>
<details class='tools'><summary>Tools</summary><div class='tmenu'>
<a href='corevex://ai?task=compare&q=%1'>Compare sources</a>
<a href='corevex://ai?task=summarize&q=%1'>Summarize results</a>
<a href='corevex://ai?task=deep&q=%1'>Deep research</a>
<a href='%2'>Open in Bing</a>
</div></details>
<button class='themebtn' onclick="document.body.classList.toggle('light')">&#9684;</button>
</div>
<div class='tabs'>
<a class='active' href='#'>All</a>
<a href='#ai-overview'>AI Overview</a>
<a href='https://www.bing.com/images/search?q=%1'>Images</a>
<a href='https://www.bing.com/videos/search?q=%1'>Videos</a>
<a href='https://www.bing.com/news/search?q=%1'>News</a>
<a href='https://www.bing.com/search?q=%1+forum'>Discussions</a>
<a href='https://www.bing.com/shop?q=%1'>Shopping</a>
</div>
<div class='layout'><div class='main'>
)HTML").arg(qUrl, bingUrl);
    // AI Overview: top 3 snippets se extractive summary, koi invented answer nahi.
    if (!results.isEmpty()) {
        QString summary;
        for (int i = 0; i < results.size() && i < 3; ++i) {
            const auto& r = results[i];
            if (!r.snippet.isEmpty()) {
                if (!summary.isEmpty()) summary += QStringLiteral(" ");
                summary += htmlEscape(r.snippet);
                summary += QStringLiteral(" <sup><a href='") + htmlEscape(r.url) + QStringLiteral("'>") + QString::number(i + 1) + QStringLiteral("</a></sup>");
            }
        }
        if (summary.isEmpty()) {
            // Snippets na mile to sirf ek title dikhana broken lagta hai —
            // top titles ki list hi summary ban jayegi.
            QStringList titles;
            for (int i = 0; i < results.size() && i < 3; ++i) titles.append(htmlEscape(results[i].title));
            summary = QStringLiteral("Top results: ") + titles.join(QStringLiteral("; "));
        }
        html += QStringLiteral("<div class='card ai' id='ai-overview'><div class='aihead'><h2>AI Overview</h2><a class='ask' href='corevex://ai?task=ask&q=%1'>Ask COREVEX &rarr;</a></div><div class='aibody'>%2</div><div class='chips'>").arg(qUrl, summary);
        for (int i = 0; i < results.size() && i < 3; ++i) {
            const auto& r = results[i];
            const QString dom = !r.displayDomain.isEmpty() ? r.displayDomain : QUrl(r.url).host();
            const QString fav = QStringLiteral("https://www.google.com/s2/favicons?domain=") + dom + QStringLiteral("&sz=32");
            html += QStringLiteral("<a class='chip' href='%1'><img src='%2' /><span>%3</span></a>").arg(htmlEscape(r.url), fav, htmlEscape(dom));
        }
        html += QStringLiteral("</div></div>");
    }
    if (results.isEmpty()) {
        html += QStringLiteral("<div class='card empty'>Koi result nahi mila. Spelling check karke dobara search karo.</div>");
    } else {
        for (const auto& r : results) {
            const QString dom = !r.displayDomain.isEmpty() ? r.displayDomain : QUrl(r.url).host();
            const QString fav = QStringLiteral("https://www.google.com/s2/favicons?domain=") + dom + QStringLiteral("&sz=32");
            QString path = QUrl(r.url).path();
            if (path.startsWith('/')) path = path.mid(1);
            path.replace('/', QStringLiteral(" \u203a "));
            if (path.size() > 60) path = path.left(60) + QStringLiteral("...");
            const QString rpath = path.isEmpty() ? dom : dom + QStringLiteral(" \u203a ") + htmlEscape(path);
            html += QStringLiteral("<a class='result' href='%1'><div class='rpath'><img src='%2' /><span>%3</span></div><div class='rtitle'>%4</div>").arg(htmlEscape(r.url), fav, rpath, htmlEscape(r.title));
            if (!r.snippet.isEmpty())
                html += QStringLiteral("<div class='rsnip'>%1</div>").arg(htmlEscape(r.snippet));
            html += QStringLiteral("</a>");
        }
    }
    html += QStringLiteral("</div><div class='side'>");
    // Research tools
    html += QStringLiteral("<div class='card'><h3>Research tools</h3>"
        "<a class='toolrow' href='corevex://ai?task=compare&q=%1'><span>Compare sources</span><span>&rarr;</span></a>"
        "<a class='toolrow' href='corevex://ai?task=deep&q=%1'><span>Deep research</span><span>&rarr;</span></a>"
        "<a class='toolrow' href='corevex://ai?task=summarize&q=%1'><span>Summarize results</span><span>&rarr;</span></a></div>").arg(qUrl);
    // Related searches: top result titles se, click karne par us phrase par search hoga.
    html += QStringLiteral("<div class='card rel'><h3>Related searches</h3>");
    int relCount = 0;
    for (const auto& r : results) {
        if (relCount >= 4) break;
        const QString relQ = QString::fromUtf8(QUrl::toPercentEncoding(r.title));
        html += QStringLiteral("<a href='corevex://search?q=%1'><span>&rarr;</span><span>%2</span></a>").arg(relQ, htmlEscape(r.title));
        ++relCount;
    }
    html += QStringLiteral("</div>");
    // Sources
    html += QStringLiteral("<div class='card'><h3>Sources</h3>");
    for (int i = 0; i < results.size() && i < 5; ++i) {
        const auto& r = results[i];
        const QString dom = !r.displayDomain.isEmpty() ? r.displayDomain : QUrl(r.url).host();
        html += QStringLiteral("<a class='srcrow' href='%1'><span class='srcnum'>%2</span><span class='srcdom'>%3</span><span class='srcurl'>%4</span></a>").arg(htmlEscape(r.url), QString::number(i + 1), htmlEscape(dom), htmlEscape(dom));
    }
    html += QStringLiteral("</div></div></div></div></body></html>");
    return html;
}

QString searchLoadingHtml(const QString& query) {
    return QStringLiteral(R"HTML(
<html><head><meta charset='utf-8'><title>Searching…</title><style>
body{margin:0;background:#0B0C0F;color:#F4F4F5;font-family:'Segoe UI',sans-serif;display:flex;align-items:center;justify-content:center;min-height:100vh;}
.box{text-align:center;}
.spin{width:44px;height:44px;border:4px solid #2A2D35;border-top-color:#FF5A1F;border-radius:50%;margin:0 auto 20px;animation:sp 1s linear infinite;}
@keyframes sp{to{transform:rotate(360deg);}}
h2{font-size:20px;} p{color:#A1A1AA;}
</style></head><body><div class='box'><div class='spin'></div><h2>Searching…</h2><p>%1</p></div></body></html>
)HTML").arg(htmlEscape(query));
}

QString searchErrorHtml(const QString& query, const QString& error) {
    const QString qUrl = QString::fromUtf8(QUrl::toPercentEncoding(query));
    return QStringLiteral(R"HTML(
<html><head><meta charset='utf-8'><title>Search failed</title><style>
body{margin:0;background:#0B0C0F;color:#F4F4F5;font-family:'Segoe UI',sans-serif;}
.wrap{max-width:800px;margin:0 auto;padding:48px 24px;}
.card{background:#14161B;border:1px solid #2A2D35;border-radius:12px;padding:28px;margin-top:20px;line-height:1.7;color:#A1A1AA;}
a{color:#FF5A1F;font-weight:700;}
</style></head><body><div class='wrap'>
<div class='card'><b style='color:#F4F4F5;font-size:18px;'>Search abhi nahi ho paya</b><br/>%1<br/><br/>
<a href='corevex://search?q=%2'>Retry search</a> &nbsp;·&nbsp; <a href='https://www.bing.com/search?q=%2'>Open in Bing</a></div>
</div></body></html>
)HTML").arg(htmlEscape(error), qUrl);
}

// --- Fetcher ---------------------------------------------------------------
CustomSearch::CustomSearch(QObject* parent) : QObject(parent) {
    nam_ = new QNetworkAccessManager(this);
    timer_ = new QTimer(this);
    timer_->setSingleShot(true);
    timer_->setInterval(12000);
    connect(timer_, &QTimer::timeout, this, &CustomSearch::onTimeout);
}

void CustomSearch::search(const QString& query) {
    if (reply_) { reply_->abort(); reply_ = nullptr; }
    timer_->stop();
    query_ = query.trimmed();
    if (query_.isEmpty()) { emit searchFailed(query_, QStringLiteral("Empty query")); return; }
    // Source order: Settings me chuna engine pehle, phir baaki.
    // Google ko source nahi banaya — uska HTML fetch bhi block hota hai,
    // isliye usko pehle rakhna sirf time waste karta.
    const QString pref = searchEngineId();
    // v3.1: DuckDuckGo REMOVED — user ko DDG ki aadat nahi, aur DDG par
    // bhi bot check aa raha tha. Sources ab: Bing (familiar) -> Brave
    // -> Mojeek. Koi bhi source DuckDuckGo nahi hai, isliye results page
    // par bhi DDG ka naam kabhi nahi aayega.
    QList<Source> all = {
        {QStringLiteral("Mojeek"), QStringLiteral("https://www.mojeek.com/search?q=%1"), 4},
        {QStringLiteral("Brave"), QStringLiteral("https://search.brave.com/search?q=%1"), 3},
        {QStringLiteral("Bing"), QStringLiteral("https://www.bing.com/search?q=%1"), 2},
    };
    sources_.clear();
    // v4.6 LINKS FIX: Mojeek pehle — uske links direct hote hain, koi
    // ck redirect nahi, isliye click karne par seedha asli site khulti hai.
    // Brave doosra, Bing aakhri (ck links sirf fallback me).
    // Settings me Brave/Bing chuna ho to wahi pehle.
    for (const auto& s : all)
        if ((pref == QStringLiteral("brave") && s.name == QStringLiteral("Brave")) ||
            (pref == QStringLiteral("mojeek") && s.name == QStringLiteral("Mojeek")))
            sources_.append(s);
    // Bing pref ko pehle nahi rakha — uske ck links click par aksar
    // khulte nahi, isliye Bing hamesha aakhri fallback hai, pehla nahi.
    for (const auto& s : all) {
        bool dup = false;
        for (const auto& e : sources_) if (e.name == s.name) dup = true;
        if (!dup) sources_.append(s);
    }
    sourceIndex_ = 0;
    tryNextSource();
}

void CustomSearch::tryNextSource() {
    if (sourceIndex_ >= sources_.size()) {
        emit searchFailed(query_, QStringLiteral("Saare search sources ne jawab nahi diya. Internet check karke retry karo — koi bot page nahi dikhaya gaya."));
        return;
    }
    startSource(sources_[sourceIndex_]);
}

void CustomSearch::startSource(const Source& source) {
    const QByteArray encBytes = QUrl::toPercentEncoding(query_);
    const QString encoded = QString::fromUtf8(encBytes);
    QNetworkRequest req(QUrl(source.urlTemplate.arg(encoded)));
    // Normal Chrome jaisa header — bot detection se bachne ke liye
    // browser jaisa request, par page WebEngine me load nahi hota.
    req.setHeader(QNetworkRequest::UserAgentHeader,
                  QStringLiteral("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/126.0.0.0 Safari/537.36"));
    req.setRawHeader("Accept", "text/html,application/xhtml+xml");
    req.setRawHeader("Accept-Language", "en-US,en;q=0.9");
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);  // Qt6: FollowRedirectsAttribute Qt5 ka tha
    reply_ = nam_->get(req);
    connect(reply_, &QNetworkReply::finished, this, &CustomSearch::onReplyFinished);
    timer_->start();
}

void CustomSearch::onTimeout() {
    if (reply_) reply_->abort();  // onReplyFinished me agla source try hoga
}

void CustomSearch::onReplyFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply || reply != reply_) return;
    timer_->stop();
    reply_ = nullptr;
    const Source source = sources_.value(sourceIndex_);
    const QByteArray body = reply->readAll();
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const bool netError = reply->error() != QNetworkReply::NoError;
    reply->deleteLater();

    const QString text = QString::fromUtf8(body);
    const QString lower = text.toLower();
    // Bot/CAPTCHA page ka pata: ye phrases aaye to source ko fail maano,
    // user ko bot page kabhi mat dikhao — seedha agla source try karo.
    const bool isBotPage = lower.contains(QStringLiteral("captcha")) ||
                           lower.contains(QStringLiteral("prove you are human")) ||
                           lower.contains(QStringLiteral("verify you are human")) ||
                           lower.contains(QStringLiteral("are you a robot")) ||
                           lower.contains(QStringLiteral("unusual traffic"));
    QList<SearchResult> results;
    if (!netError && status >= 200 && status < 400 && !isBotPage)
        results = parseResults(text, source.parser);
    if (!results.isEmpty()) {
        emit resultsReady(query_, results, source.name);
        return;
    }
    // Khaali ya bot ya error — agla source.
    ++sourceIndex_;
    tryNextSource();
}

QList<SearchResult> CustomSearch::parseResults(const QString& html, int parser) const {
    QList<SearchResult> out;
    auto cleanUrl = [](QString u) {
        u = u.trimmed();
        u.replace(QStringLiteral("&amp;"), QStringLiteral("&"));
        u.replace(QStringLiteral("&#38;"), QStringLiteral("&"));
        if (u.startsWith(QStringLiteral("//"))) u = QStringLiteral("https:") + u;
        return u;
    };
    // v4.6 Mojeek specific: direct links, simple HTML.
    if (parser == 4) {
        static const QRegularExpression mojeekRe(QStringLiteral("<a[^>]*href=\"([^\"]+)\"[^>]*>(.*?)</a>"));
        QStringList seen;
        for (auto it = mojeekRe.globalMatch(html); it.hasNext() && out.size() < 10;) {
            auto m = it.next();
            QString url = cleanUrl(m.captured(1));
            if (!url.startsWith(QStringLiteral("http"))) continue;
            if (url.contains(QStringLiteral("mojeek.com"))) continue;
            const QString title = stripTags(m.captured(2));
            if (title.size() < 4 || seen.contains(url)) continue;
            seen.append(url);
            QString snip;
            const QString tail = html.mid(m.capturedEnd(), 1000);
            static const QRegularExpression pRe(QStringLiteral("<p[^>]*>(.*?)</p>"));
            auto pm = pRe.match(tail);
            if (pm.hasMatch()) snip = stripTags(pm.captured(1));
            out.append({title, url, snip, QUrl(url).host()});
        }
        if (!out.isEmpty()) return out;
    }
    // v4.5 Brave specific: result-header links direct hote hain, snippet
    // snippet-description me hota hai. Generic parser par girne se pehle
    // ye try karo, taaki navigation links result na ban jayein.
    if (parser == 3) {
        static const QRegularExpression braveRe(QStringLiteral("<a[^>]*class=\"[^\"]*result-header[^\"]*\"[^>]*href=\"([^\"]+)\"[^>]*>(.*?)</a>"));
        static const QRegularExpression braveSnipRe(QStringLiteral("class=\"[^\"]*snippet-description[^\"]*\"[^>]*>(.*?)</"));
        QStringList seen;
        for (auto it = braveRe.globalMatch(html); it.hasNext() && out.size() < 10;) {
            auto m = it.next();
            QString url = cleanUrl(m.captured(1));
            if (!url.startsWith(QStringLiteral("http")) || seen.contains(url)) continue;
            const QString title = stripTags(m.captured(2));
            if (title.size() < 4) continue;
            seen.append(url);
            QString snip;
            const QString tail = html.mid(m.capturedEnd(), 1200);
            auto pm = braveSnipRe.match(tail);
            if (pm.hasMatch()) snip = stripTags(pm.captured(1));
            const QString dom = QUrl(url).host();
            out.append({title, url, snip, dom});
        }
        if (!out.isEmpty()) return out;
    }
    if (parser == 2) {
        // Bing: b_algo blocks, cite se asli domain (ck link ka host bing.com nahi).
        static const QRegularExpression blockRe(QStringLiteral("<li[^>]*class=\"[^\"]*b_algo[^\"]*\"[^>]*>(.*?)</li>"));
        static const QRegularExpression linkRe(QStringLiteral("<h2[^>]*>\\s*<a[^>]*href=\"([^\"]+)\"[^>]*>(.*?)</a>"));
        static const QRegularExpression snipRe(QStringLiteral("<p>(.*?)</p>"));
        static const QRegularExpression citeRe(QStringLiteral("<cite[^>]*>(.*?)</cite>"));
        for (auto bit = blockRe.globalMatch(html); bit.hasNext() && out.size() < 10;) {
            const QString block = bit.next().captured(1);
            auto lm = linkRe.match(block);
            if (!lm.hasMatch()) continue;
            const QString url = cleanUrl(lm.captured(1));
            if (!url.startsWith(QStringLiteral("http"))) continue;
            QString snip;
            {
                static const QRegularExpression capRe(QStringLiteral("<div[^>]*class=\"[^\"]*b_caption[^\"]*\"[^>]*>.*?<p[^>]*>(.*?)</p>"));
                static const QRegularExpression spanRe(QStringLiteral("<span[^>]*class=\"[^\"]*b_snippet[^\"]*\"[^>]*>(.*?)</span>"));
                auto cm2 = capRe.match(block);
                auto sm2 = spanRe.match(block);
                if (cm2.hasMatch()) snip = stripTags(cm2.captured(1));
                else if (sm2.hasMatch()) snip = stripTags(sm2.captured(1));
                else { auto sm = snipRe.match(block); if (sm.hasMatch()) snip = stripTags(sm.captured(1)); }
            }
            QString dom;
            auto cm = citeRe.match(block);
            if (cm.hasMatch()) {
                dom = stripTags(cm.captured(1)).trimmed();
                // cite "en.wikipedia.org › wiki › Hello" -> pehla hissa hi domain
                const int sep = dom.indexOf(QStringLiteral(" › "));
                if (sep >= 0) dom = dom.left(sep);
                dom = dom.trimmed();
            }
            if (dom.isEmpty()) dom = QUrl(url).host();
            // ck link ka host bing.com ho to bhi display domain cite wala rahega,
            // click karne par ck redirect asli site par le jayega.
            out.append({stripTags(lm.captured(2)), url, snip, dom});
        }
        if (!out.isEmpty()) return out;
    }
    // Generic parser (fallback): sab <a href="http...">, engine internal links hatao.
    static const QRegularExpression linkRe(QStringLiteral("<a[^>]*href=\"([^\"]+)\"[^>]*>(.*?)</a>"));
    QStringList seen;
    for (auto it = linkRe.globalMatch(html); it.hasNext() && out.size() < 10;) {
        auto m = it.next();
        QString url = cleanUrl(m.captured(1));
        if (!url.startsWith(QStringLiteral("http"))) continue;
        if (url.contains(QStringLiteral("bing.com/search")) || url.contains(QStringLiteral("brave.com/search")) ||
            url.contains(QStringLiteral("mojeek.com/search")) || url.contains(QStringLiteral("search.brave.com")) ||
            url.contains(QStringLiteral("javascript:"))) continue;
        const QString title = stripTags(m.captured(2));
        if (title.size() < 4 || seen.contains(url)) continue;
        seen.append(url);
        QString snip;
        const QString tail = html.mid(m.capturedEnd(), 800);
        static const QRegularExpression pRe(QStringLiteral("<p[^>]*>(.*?)</p>"));
        auto pm = pRe.match(tail);
        if (pm.hasMatch()) snip = stripTags(pm.captured(1));
        out.append({title, url, snip, QUrl(url).host()});
    }
    return out;
}

}  // namespace corevex
