#pragma once
// ============================================================================
// SECTION 4 — NEW-TAB PAGE (inspiration screenshot jaisa, par kaam karta hua)
// ----------------------------------------------------------------------------
// Kya karta hai:
//   New tab ka poora HTML banata hai: bada "Search. Browse. Think.",
//   kaam karta hua search box, chips, Quick Access tiles, Recent
//   Activity aur quote card.
// Purane code se farq (yehi improvement hai):
//   * Search box sirf dikhne ke liye nahi hai — ye ek real <form> hai
//     jo Google search par jaata hai (action + input name="q").
//   * Quick tiles sirf <div> nahi hain — ye real <a href> links hain,
//     click karne par site khulti hai.
//   * Chips search box me text daalte hain (JS), phir Enter dabane
//     par search hoti hai.
//   * Icons Assets resolver se aate hain, hardcoded E: path se nahi.
// ============================================================================
#include <QString>

namespace corevex {

// New-tab page ka HTML. Caller is HTML ko QWebEngineView::setHtml se
// dikhata hai. HTML me koi external network call nahi hai, sirf
// local SVG icons aur Google search form hai.
QString newTabPageHtml();

}  // namespace corevex
