#pragma once
// ============================================================================
// SECTION 5 — PERSISTENT STORE (SQLite: bookmarks + history)
// ----------------------------------------------------------------------------
// Kya karta hai:
//   Bookmarks aur history ko SQLite file me save karta hai, taaki app
//   band karke dobara kholne par bhi data mile. Sirf metadata save
//   hota hai (title + URL + time) — poora page content nahi.
// Kyun zaroori hai:
//   Purana code bookmarks/history sirf memory me rakhta tha (aur
//   addBookmark() to khaali tha), isliye restart par sab gayab ho
//   jaata tha. Ye module us gap ko band karta hai.
// File kahan:
//   QStandardPaths::AppLocalDataLocation/corevex.db (Windows par
//   %LOCALAPPDATA%/COREVEX/browser/corevex.db ke aas-paas).
// ============================================================================
#include <QDateTime>
#include <QList>
#include <QString>
#include <QSqlDatabase>

namespace corevex {

struct Bookmark { QString title; QString url; };
struct HistoryItem { QString title; QString url; QDateTime time; };

class Store {
public:
    Store();
    bool open();  // false = app phir bhi chalegi, persistence skip hogi.
    bool isOpen() const { return open_; }

    bool addBookmark(const QString& title, const QString& url);
    bool removeBookmark(const QString& url);
    bool isBookmarked(const QString& url) const;
    QList<Bookmark> bookmarks() const;

    bool addHistory(const QString& title, const QString& url);
    QList<HistoryItem> history(int limit = 100) const;
    bool clearHistory();

private:
    bool createTables();
    QSqlDatabase db_;
    bool open_ = false;
};

}  // namespace corevex
