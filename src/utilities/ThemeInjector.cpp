// ============================================================================
// SECTION 3 (impl) — COREVEX THEME INJECTION
// ============================================================================
#include "ThemeInjector.h"

#include <QWebEngineProfile>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QStandardPaths>

namespace corevex {

void installCorevexThemeScripts() {
    auto* profile = QWebEngineProfile::defaultProfile();
    if (!profile) return;
    // Production persistence: cookies/login/cache restart par bhi rahen,
    // warna har baar login karna padta (pehle default profile par ye
    // guarantee nahi thi).
    profile->setPersistentStoragePath(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + QStringLiteral("/webengine"));
    profile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);
    profile->setPersistentCookiesPolicy(QWebEngineProfile::AllowPersistentCookies);

    // --- 3A. Search results dark theme (Google + Bing + Brave) — v4.2 clean list ---
    // Qt 6.7 me QWebEngineScript ka koi setUrlPattern() nahi hai, isliye
    // JS khud hostname check karta hai. Generic CSS sab engines par
    // chalta hai (body/link/card/input), Google-specific selectors
    // alag se hain. COREVEX Search results page apna HTML hai, isliye us par theme
    // ki zaroorat hi nahi — ye injection sirf seedha khole hue engine
    // pages (Google/Bing) par lagta hai.
    QWebEngineScript searchTheme;
    searchTheme.setName(QStringLiteral("CorevexSearchTheme"));
    searchTheme.setInjectionPoint(QWebEngineScript::DocumentReady);
    searchTheme.setWorldId(QWebEngineScript::ApplicationWorld);
    searchTheme.setRunsOnSubFrames(false);
    QString css = QStringLiteral(
        "body{background:#0B0C0F !important;color:#F4F4F5 !important;}"
        "a{color:#FF5A1F !important;} a:visited{color:#FF8A50 !important;}"
        "div, span, p, h3, td{color:#F4F4F5 !important;}"
        /* v4.2: Google results ko card nahi, saaf list rakho — card border
           + padding hi UI ko trash dikhata tha. Sirf neeche halki line. */
        ".g, .MjjYud, .result, .results, .b_algo, #b_results li{ background:transparent !important;"
        "border:none !important;border-bottom:1px solid #2A2D35 !important;border-radius:0 !important;"
        "padding:12px 0 !important;margin-bottom:8px !important;}"
        "#top_nav, #hdtb, .appbar, #searchform{background:#14161B !important;border-bottom:1px solid #2A2D35 !important;}"
        "#search, #center_col, #links, #b_content{ background:transparent !important;}"
        "input, textarea{background:#1B1E24 !important;color:#F4F4F5 !important;"
        "border:1px solid #2A2D35 !important;border-radius:20px !important;}"
        "button, input[type=submit]{background:#FF5A1F !important;color:#fff !important;border:none !important;}"
        ".aajZCb, .VuuXrf, .result__snippet, .b_caption{color:#A1A1AA !important;}");
    QString cssEscaped = css;
    cssEscaped.replace(QStringLiteral("'"), QStringLiteral("\\'"));
    searchTheme.setSourceCode(
        QStringLiteral("(function(){var h=location.hostname;"
                       "var ok=h.endsWith('google.com')||"
                       "h.endsWith('bing.com')||h.endsWith('search.brave.com');"
                       "if(!ok)return;"
                       "var s=document.createElement('style');"
                       "s.textContent=%1;document.documentElement.appendChild(s);})();")
            .arg(QStringLiteral("'") + cssEscaped + QStringLiteral("'")));
    profile->scripts()->insert(searchTheme);

    // --- 3A2. Normal Chrome user agent (bot/CAPTCHA kam ho) --------------
    // Qt ka default UA ("QtWebEngine/...") Google ko bot jaisa lagta hai.
    // Asli Chrome jaisa UA bhejne se search engines page ko normal
    // browser samajhte hain. Ye CAPTCHA ko guarantee se nahi rokta
    // (Google phir bhi kabhi-kabhi check kar sakta hai), par isse +
    // COREVEX Search se problem practically khatam ho jaati hai.
    profile->setHttpUserAgent(QStringLiteral(
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
        "(KHTML, like Gecko) Chrome/126.0.0.0 Safari/537.36"));

    // --- 3B. Dark scrollbar (har site par, sirf scrollbar) ------------
    // Ye purane BrowserWindow.cpp wale idea ka saaf version hai: sirf
    // scrollbar ka rang badalta hai, site ka layout nahi chhedta.
    QWebEngineScript scrollbar;
    scrollbar.setName(QStringLiteral("CorevexDarkScrollbar"));
    scrollbar.setInjectionPoint(QWebEngineScript::DocumentReady);
    scrollbar.setWorldId(QWebEngineScript::ApplicationWorld);
    scrollbar.setRunsOnSubFrames(true);
    scrollbar.setSourceCode(
        QStringLiteral("let s=document.createElement('style');"
                       "s.innerHTML='::-webkit-scrollbar{width:12px;height:12px;}"
                       "::-webkit-scrollbar-track{background:#0B0C0F;}"
                       "::-webkit-scrollbar-thumb{background:#2A2D35;border:3px solid #0B0C0F;border-radius:6px;}"
                       "::-webkit-scrollbar-thumb:hover{background:#FF5A1F;}"
                       "::-webkit-scrollbar-corner{background:#0B0C0F;}';"
                       "if(document.head) document.head.appendChild(s);"));
    profile->scripts()->insert(scrollbar);
}

}  // namespace corevex
