// ============================================================================
// SECTION 4 (impl) — NEW-TAB PAGE (inspiration jaisa, ab background + icons ke saath)
// ----------------------------------------------------------------------------
// Fix jo screenshots se mila:
//   * Hero waves background pehle Q_UNUSED tha — ab body ka background
//     hero_waves.svg (data URI) hai, right side par waves dikhengi,
//     left side dark rahega, exactly inspiration jaisa.
//   * Quick tiles ke icons pehle file:// URL the, WebEngine ne block
//     kar diye the (broken-image icon). Ab sab icons data URI hain,
//     isliye chahe app kisi bhi folder se chale, icons dikhenge.
//   * Recent Activity ka Hinglish placeholder hataya — ab professional
//     English text hai, koi fake history nahi dikhata.
//   * HTML me <title>New Tab</title> hai, taaki tab par "google.com"
//     na likha aaye (pehle baseUrl google.com tha, isliye tab ka naam
//     google.com ho jaata tha).
// ============================================================================
#include "NewTabPage.h"

#include "../utilities/Assets.h"
#include "../search/SearchEngine.h"
#include <QFile>
#include <QSettings>
#include <QStandardPaths>
#include <QBuffer>
#include <QImage>
#include <QPainter>

namespace corevex {

QString newTabPageHtml() {
    // --- 4A. Icons + background, sab data URI (WebEngine-safe) -----------
    // User ka bg.png — kahan bhi ho, dhoondho (pichhli baar bg change
    // nahi hua kyunki sirf ek path try ho raha tha):
    //   assets/backgrounds/bg.png  (sahi jagah)
    //   assets/background/bg.png   (singular, agar user ne aise dala ho)
    //   assets/bg.png
    // Na mile to hero_waves.svg fallback. Har path Assets resolver se
    // guzarta hai, jo exe folder, ../assets aur current folder — teeno
    // jagah check karta hai, isliye run karne ke tareeke se farq nahi padta.
    // BG ka sahi order (pichhli baar svg isliye aaya kyunki bg.png
    // Downloads me tha, assets me nahi — aur code sirf assets dhoondhta tha):
    //   1) Settings me chuni hui image (agar user ne chooser se chuni ho)
    //   2) Downloads/bg.png (tumhara bg.png yahi dikha hai, 2.5 MB)
    //   3) assets/backgrounds/bg.png / background/bg.png / bg.png
    //   4) fallback hero_waves.svg
    QString bg;
    // WHITE PAGE FIX: 2.5 MB PNG ko seedha base64 me daalne se HTML
    // ~3.3 MB ka ho jaata tha, aur WebEngine usko load karne me white
    // screen dikhata tha. Isliye har bg image ko pehle JPEG me
    // downscale karo (max 1920px, quality 82) — ~280 KB, jo turant load hota hai.
    // bg_small.jpg pehle se hi compressed version hai, isliye wo pehle try hota hai.
    auto scaledImageDataUri = [](const QString& path) -> QString {
        QImage img(path);
        if (img.isNull()) return QString();
        if (img.width() > 1920) img = img.scaledToWidth(1920, Qt::SmoothTransformation);
        // PNG alpha ko black background par flatten karo, warna JPEG me ajeeb dikhega.
        if (img.hasAlphaChannel()) {
            QImage rgb(img.size(), QImage::Format_RGB32);
            rgb.fill(Qt::black);
            QPainter painter(&rgb);
            painter.drawImage(0, 0, img);
            painter.end();
            img = rgb;
        }
        QByteArray bytes;
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::WriteOnly);
        if (!img.save(&buffer, "JPEG", 82)) return QString();
        return QStringLiteral("data:image/jpeg;base64,") + QString::fromLatin1(bytes.toBase64());
    };
    auto fileDataUri = [&](const QString& path) -> QString {
        QString scaled = scaledImageDataUri(path);
        if (!scaled.isEmpty()) return scaled;
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly)) return QString();
        const QByteArray bytes = f.readAll();
        if (bytes.isEmpty()) return QString();
        QString mime = QStringLiteral("image/png");
        if (path.endsWith(QStringLiteral(".jpg"), Qt::CaseInsensitive) || path.endsWith(QStringLiteral(".jpeg"), Qt::CaseInsensitive)) mime = QStringLiteral("image/jpeg");
        else if (path.endsWith(QStringLiteral(".svg"), Qt::CaseInsensitive)) mime = QStringLiteral("image/svg+xml");
        return QStringLiteral("data:") + mime + QStringLiteral(";base64,") + QString::fromLatin1(bytes.toBase64());
    };
    {
        QSettings s(QStringLiteral("COREVEX"), QStringLiteral("Browser"));
        const QString chosen = s.value(QStringLiteral("background/imagePath")).toString();
        if (!chosen.isEmpty()) bg = fileDataUri(chosen);
    }
    if (bg.isEmpty()) {
        const QString dlBg = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + QStringLiteral("/bg.png");
        bg = fileDataUri(dlBg);
    }
    if (bg.isEmpty()) {
        // v3.1 FIX — pehle bg_small.jpg jeet jaata tha, isliye user ka naya
        // bg.png kabhi dikhta hi nahi tha ("bg change nahi hua"). Ab order:
        // bg.png (tumhara latest image) PEHLE, bg_small.jpg sirf fallback.
        for (const QString& candidate : {QStringLiteral("backgrounds/bg.png"),
                                          QStringLiteral("background/bg.png"),
                                          QStringLiteral("bg.png")}) {
            QString resPath = QStringLiteral(":/assets/") + candidate;
            bg = scaledImageDataUri(resPath);
            if (bg.isEmpty()) bg = assetDataUri(candidate);
            if (!bg.isEmpty()) break;
        }
    }
    if (bg.isEmpty()) {
        bg = assetDataUri(QStringLiteral("backgrounds/bg_small.jpg"));
    }
    if (bg.isEmpty()) bg = assetDataUri(QStringLiteral("backgrounds/hero_waves.svg"));
    const QString git = assetDataUri(QStringLiteral("icons/quick/quick_github.svg"));
    const QString yt = assetDataUri(QStringLiteral("icons/quick/quick_youtube.svg"));
    const QString lc = assetDataUri(QStringLiteral("icons/quick/quick_leetcode.svg"));
    const QString goog = assetDataUri(QStringLiteral("icons/quick/quick_google.svg"));
    const QString gpt = assetDataUri(QStringLiteral("icons/quick/quick_chatgpt.svg"));
    const QString in = assetDataUri(QStringLiteral("icons/quick/quick_linkedin.svg"));

    QString html = QStringLiteral(R"HTML(
<html><head><meta charset='utf-8'><title>New Tab</title><style>
body{margin:0;background-color:#0B0C0F;color:#F4F4F5;font-family:'Segoe UI',sans-serif;overflow-x:hidden;
)HTML");
    if (!bg.isEmpty())
        html += QStringLiteral("background-image:url('%1');background-size:cover;background-position:center center;background-repeat:no-repeat;").arg(bg);
    html += QStringLiteral(R"HTML(}
.glow{position:fixed;right:-120px;top:-80px;width:640px;height:640px;background:transparent;pointer-events:none;z-index:0;}
.scrim{position:fixed;inset:0;background:linear-gradient(180deg,rgba(0,0,0,.68),rgba(0,0,0,.45) 50%,rgba(0,0,0,.68));pointer-events:none;z-index:0;}
.container{display:flex;flex-direction:column;align-items:center;padding-top:4vh;padding-bottom:24px;box-sizing:border-box;position:relative;z-index:1;}
h1{font-size:clamp(32px,6vw,56px);margin:0;font-weight:700;letter-spacing:-1px;line-height:1.1;white-space:normal;word-break:keep-all;text-shadow:0 3px 18px rgba(0,0,0,1),0 0 6px rgba(0,0,0,1);background:rgba(0,0,0,.35);padding:12px 20px;border-radius:16px;}
h1 span{color:#FF5A1F;}
.subtitle{color:#FFFFFF;font-size:16px;margin:12px 0 36px;text-shadow:0 2px 10px rgba(0,0,0,1);background:rgba(0,0,0,.35);padding:8px 16px;border-radius:12px;}
.search-box{background:rgba(20,22,27,.94);border:1px solid #2A2D35;border-radius:30px;padding:6px 6px 6px 24px;width:680px;max-width:92vw;display:flex;align-items:center;box-shadow:0 12px 40px rgba(0,0,0,.6);transition:border-color .2s, box-shadow .2s;}
.search-box:focus-within{border-color:#FF5A1F;box-shadow:0 12px 40px rgba(255,90,31,.25);}
.search-box input{background:transparent;border:none;color:#fff;font-size:15px;flex:1;outline:none;}
.search-btn{background:#FF5A1F;border:none;border-radius:24px;width:44px;height:44px;color:#fff;font-size:20px;cursor:pointer;}
.chips{display:flex;gap:12px;margin-top:20px;flex-wrap:wrap;justify-content:center;}
.chip{background:rgba(27,30,36,.7);border:1px solid #2A2D35;border-radius:20px;padding:8px 16px;font-size:13px;color:#A1A1AA;cursor:pointer;}
.chip:hover{color:#fff;border-color:#FF5A1F;}
.qa{margin-top:28px;width:680px;max-width:92vw;}
.qa-title{color:#A1A1AA;font-size:14px;margin-bottom:14px;font-weight:600;}
.grid{display:flex;gap:14px;flex-wrap:wrap;}
.tile{background:rgba(20,22,27,.90);border:1px solid #2A2D35;border-radius:14px;width:88px;height:80px;display:flex;flex-direction:column;align-items:center;justify-content:center;text-decoration:none;color:#A1A1AA;box-shadow:0 4px 16px rgba(0,0,0,.35);transition:transform .15s, border-color .15s;}
.tile:hover{color:#fff;border-color:#FF5A1F;transform:translateY(-2px);}
.tile img{width:24px;height:24px;margin-bottom:8px;}
.tile span{font-size:12px;}
.bottom{display:flex;gap:20px;width:680px;max-width:92vw;margin-top:16px;flex-wrap:wrap;}
.card{background:rgba(20,22,27,.90);border:1px solid #2A2D35;border-radius:14px;padding:16px 20px;flex:1;min-width:240px;box-shadow:0 6px 20px rgba(0,0,0,.35);}
.card:hover{border-color:#3A3E4A;}
.card p{color:#A1A1AA;font-size:13px;line-height:1.6;}
.quote{font-style:italic;font-size:15px;}
.quote b{color:#FF5A1F;font-style:normal;display:block;margin-top:10px;font-size:13px;}
</style></head><body>
<div class='glow'></div>
<div class='scrim'></div>
<div class='container'>
<h1>Search. Browse. <span>Think.</span></h1>
<div class='subtitle'>Your AI-powered browser for deeper research, smarter answers</div>
)HTML");
    // Form ka action COREVEX Search (corevex://search) se aata hai,
    // isliye raw HTML string ko yahan todkar dynamic URL joda gaya hai.
    html += QStringLiteral("<form class='search-box' action='%1' method='get'>")
                .arg(searchEngineBaseUrl(searchEngineId()));
    html += QStringLiteral(R"HTML(
<input id='q' name='q' type='text' placeholder='Search the web or ask anything...' autofocus />
<button class='search-btn' type='submit'>&#8594;</button>
</form>
<div class='chips'>
<span class='chip' onclick="document.getElementById('q').value='';document.getElementById('q').focus();">Research</span>
<span class='chip' onclick="document.getElementById('q').value='Summarize ';document.getElementById('q').focus();">Summarize Page</span>
<span class='chip' onclick="document.getElementById('q').value='Compare ';document.getElementById('q').focus();">Compare Sources</span>
<span class='chip' onclick="document.getElementById('q').value='Find ';document.getElementById('q').focus();">Find Information</span>
</div>
<div class='qa'><div class='qa-title'>Quick Access</div><div class='grid'>
)HTML");
    auto tile = [](const QString& url, const QString& icon, const QString& label) {
        return QStringLiteral("<a class='tile' href='%1'><img src='%2'/><span>%3</span></a>")
            .arg(url, icon, label);
    };
    html += tile(QStringLiteral("https://www.github.com"), git, QStringLiteral("GitHub"));
    html += tile(QStringLiteral("https://www.youtube.com"), yt, QStringLiteral("YouTube"));
    html += tile(QStringLiteral("https://leetcode.com"), lc, QStringLiteral("LeetCode"));
    html += tile(QStringLiteral("https://www.google.com"), goog, QStringLiteral("Google"));
    html += tile(QStringLiteral("https://chatgpt.com"), gpt, QStringLiteral("ChatGPT"));
    html += tile(QStringLiteral("https://www.linkedin.com"), in, QStringLiteral("LinkedIn"));
    html += QStringLiteral(R"HTML(
</div></div>
<div class='bottom'>
<div class='card'><div class='qa-title'>Recent Activity</div><p>No recent activity yet.<br/>Search or open a site, and it will appear in History.</p></div>
<div class='card'><div class='quote'>"The best way to predict the future is to build it."<b>- Alan Kay</b></div></div>
</div></div></body></html>
)HTML");
    return html;
}

}  // namespace corevex
