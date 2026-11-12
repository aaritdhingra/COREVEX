#pragma once
// ============================================================================
// SECTION 11 — SEARCH ENGINE (Google CAPTCHA ka asli fix)
// ----------------------------------------------------------------------------
// v4.4 TEMP (bina key) — user ne temp bina key bola hai, isliye
//   default Bing hai. Google direct sorry-block ho raha hai, isliye
//   temp me Google primary nahi hai. Google API key milte hi permanent
//   Google API version banega, tab ye temp khatam hoga.
// v4.2 — user ka faisla: Google par hi search hoga.
//   COREVEX Search fetch (v2.4–v4.1) ko band kar diya gaya, kyunki
//   uska Bing parser toota hua tha: har result ka URL www.bing.com
//   dikhta tha aur snippet khaali rehta tha. Ab address bar, new-tab
//   form aur Search page — teeno — seedha Google ke asli search page
//   par jaate hain, aur ThemeInjector us page ko COREVEX dark + orange
//   theme me dikhata hai. DuckDuckGo COREVEX me ab kahin nahi hai.
//   * Default Google hai; Settings me Bing / Brave / Mojeek bhi hai.
//   * Har engine ka query parameter "q" hai.
// ============================================================================
#include <QString>
#include <QUrl>

namespace corevex {

QString searchEngineId();          // "bing" (TEMP default, v4.4) / "google" / "brave" / "mojeek"
void setSearchEngineId(const QString& id);
QString searchEngineName(const QString& id);  // display name
QUrl searchUrlForEngine(const QString& query, const QString& id);
QUrl searchUrl(const QString& query);  // saved engine ke hisaab se
QString searchEngineBaseUrl(const QString& id);  // new-tab form action ke liye

}  // namespace corevex
