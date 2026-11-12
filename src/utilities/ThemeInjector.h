#pragma once
// ============================================================================
// SECTION 3 — COREVEX THEME INJECTION (Google search bhi apni theme me)
// ----------------------------------------------------------------------------
// Kya karta hai:
//   Jab user Google par search karta hai, to results page ka safed
//   theme COREVEX ke dark theme me badal deta hai: background #0B0C0F,
//   cards #14161B, text #F4F4F5, links/buttons orange #FF5A1F, aur
//   dark scrollbar. Ye page ka content ya functionality nahi badalta,
//   sirf CSS inject karta hai.
// Kaise kaam karta hai:
//   Qt WebEngine ka QWebEngineScript, DocumentReady par, sirf
//   google.com ke liye (urlPattern) chalaya jaata hai. Script ek
//   <style> tag daalta hai — isliye Google ka apna JS/search kaam
//   waise hi chalta rehta hai, sirf rang COREVEX jaise ho jaate hain.
// Limit:
//   Google apna HTML kabhi bhi badal sakta hai, isliye CSS me
//   !important aur broad selectors use kiye gaye hain, taaki zyada
//   se zyada elements cover hon. Agar Google layout badle, to theme
//   partial ho sakti hai, search kabhi nahi tootegi.
// ============================================================================
#include <QObject>

namespace corevex {

// Default WebEngine profile me theme + scrollbar scripts install karta hai.
// Ek baar app start me call karo, har tab par apne aap lagega.
void installCorevexThemeScripts();

}  // namespace corevex
