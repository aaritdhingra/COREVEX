#pragma once
// ============================================================================
// SECTION 1 — ASSET PATH RESOLUTION (FIXED: Qt resource, working dir se azad)
// ----------------------------------------------------------------------------
// Root cause jo screenshots me dikha:
//   Pehle icons sirf filesystem se milte the ("assets/..."), jo sirf tab
//   chalta tha jab app Desktop\browser se chalayi jaaye. build folder se
//   double-click karne par, ya WebEngine ke setHtml me, file:// images
//   block ho jaati thi — isliye Quick tiles par broken-image icon aaya,
//   tab close X gayab hua, aur AI button khaali orange circle dikha.
// Fix:
//   * Sab SVG/QSS ab exe ke andar Qt resource (assets/corevex.qrc) me
//     embed hain. QIcon/QPixmap ke liye ":/assets/..." path hamesha
//     kaam karega, chahe app kisi bhi folder se chale.
//   * WebEngine HTML ke liye file:// nahi, data URI use hota hai
//     (assetDataUri), kyunki WebEngine local file access block karta hai.
//   * AI button ke liye alag white sparkle icon hai, kyunki orange
//     sparkle orange button par dikhta hi nahi tha.
// ============================================================================
#include <QString>

namespace corevex {

// QIcon/QPixmap ke liye path: pehle resource, phir filesystem fallback.
QString assetPath(const QString& relativePath);

// WebEngine HTML ke liye data URI (SVG ko base64 me embed karta hai).
QString assetDataUri(const QString& relativePath);

// Purana helper, ab data URI ke liye use mat karo (sirf compat ke liye).
QString assetUrl(const QString& relativePath);

// corevex.qss ko resource/filesystem se padhkar app par lagata hai.
// Iske bina Pages ke buttons default grey dikhte the (QSS kabhi load
// hi nahi hua tha). main.cpp me app banne ke turant baad call karo.
void loadApplicationStyleSheet();

}  // namespace corevex
