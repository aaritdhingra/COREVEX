// ============================================================================
// SECTION 5 (impl) — PERSISTENT STORE
// ============================================================================
#include "Store.h"

#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QVariant>

namespace corevex {

Store::Store() = default;

bool Store::open() {
    const QString dir = QStandardPaths::writableLocation(
        QStandardPaths::AppLocalDataLocation);
    QDir().mkpath(dir);
    if (QSqlDatabase::contains(QStringLiteral("corevex"))) {
        db_ = QSqlDatabase::database(QStringLiteral("corevex"));
    } else {
        db_ = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"),
                                        QStringLiteral("corevex"));
    }
    db_.setDatabaseName(dir + QStringLiteral("/corevex.db"));
    if (!db_.open()) return false;
    open_ = createTables();
    return open_;
}

bool Store::createTables() {
    QSqlQuery q(db_);
    if (!q.exec(QStringLiteral(
            "CREATE TABLE IF NOT EXISTS bookmarks ("
            "url TEXT PRIMARY KEY, title TEXT NOT NULL)"))) return false;
    if (!q.exec(QStringLiteral(
            "CREATE TABLE IF NOT EXISTS history ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, title TEXT NOT NULL, "
            "url TEXT NOT NULL, visited_at INTEGER NOT NULL)"))) return false;
    return true;
}

bool Store::addBookmark(const QString& title, const QString& url) {
    if (!open_ || url.isEmpty()) return false;
    QSqlQuery q(db_);
    q.prepare(QStringLiteral(
        "INSERT INTO bookmarks (url, title) VALUES (?, ?) "
        "ON CONFLICT(url) DO UPDATE SET title=excluded.title"));
    q.addBindValue(url);
    q.addBindValue(title.isEmpty() ? url : title);
    return q.exec();
}

bool Store::removeBookmark(const QString& url) {
    if (!open_) return false;
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("DELETE FROM bookmarks WHERE url = ?"));
    q.addBindValue(url);
    return q.exec();
}

bool Store::isBookmarked(const QString& url) const {
    if (!open_) return false;
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("SELECT 1 FROM bookmarks WHERE url = ? LIMIT 1"));
    q.addBindValue(url);
    return q.exec() && q.next();
}

QList<Bookmark> Store::bookmarks() const {
    QList<Bookmark> out;
    if (!open_) return out;
    QSqlQuery q(db_);
    if (!q.exec(QStringLiteral("SELECT title, url FROM bookmarks ORDER BY title ASC"))) return out;
    while (q.next()) out.append({q.value(0).toString(), q.value(1).toString()});
    return out;
}

bool Store::addHistory(const QString& title, const QString& url) {
    if (!open_ || url.isEmpty()) return false;
    QSqlQuery q(db_);
    q.prepare(QStringLiteral(
        "INSERT INTO history (title, url, visited_at) VALUES (?, ?, ?)"));
    q.addBindValue(title.isEmpty() ? url : title);
    q.addBindValue(url);
    q.addBindValue(QDateTime::currentSecsSinceEpoch());
    return q.exec();
}

QList<HistoryItem> Store::history(int limit) const {
    QList<HistoryItem> out;
    if (!open_) return out;
    QSqlQuery q(db_);
    q.prepare(QStringLiteral(
        "SELECT title, url, visited_at FROM history "
        "ORDER BY visited_at DESC, id DESC LIMIT ?"));
    q.addBindValue(limit);
    if (!q.exec()) return out;
    while (q.next()) {
        HistoryItem item;
        item.title = q.value(0).toString();
        item.url = q.value(1).toString();
        item.time = QDateTime::fromSecsSinceEpoch(q.value(2).toLongLong());
        out.append(item);
    }
    return out;
}

bool Store::clearHistory() {
    if (!open_) return false;
    QSqlQuery q(db_);
    return q.exec(QStringLiteral("DELETE FROM history"));
}

}  // namespace corevex
