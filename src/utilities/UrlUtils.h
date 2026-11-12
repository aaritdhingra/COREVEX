#pragma once
// ============================================================================
// SECTION 2 — ADDRESS-BAR URL RESOLUTION
// ----------------------------------------------------------------------------
// Kya karta hai:
//   Address bar ka text lekar batata hai ki ye (a) ek URL hai, ya
//   (b) ek search query hai. URL ho to seedha kholta hai, query ho to
//   Google search par bhejta hai, sahi percent-encoding ke saath.
// Kyun zaroori hai:
//   Purana code query ko bina encoding ke URL me chipkata tha, jisse
//   space / special characters wali search toot jaati thi. Ye module
//   us bug ko fix karta hai, aur javascript:/file: jaise khatarnak
//   scheme ko kabhi navigation nahi banne deta.
// ============================================================================
#include <QString>
#include <QUrl>

namespace corevex {

// Raw address-bar text -> navigation URL (kabhi khaali nahi, query
// hone par search URL ban jaata hai).
QUrl resolveAddressInput(const QString& input);

// Sirf http/https, valid host wale URL ko safe maanta hai.
bool isSafeWebUrl(const QUrl& url);

// Google search URL for a query.
QUrl googleSearchUrl(const QString& query);

}  // namespace corevex
