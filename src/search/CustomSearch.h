#pragma once
// ============================================================================
// SECTION 13 — COREVEX SEARCH (bot/human check ka asli fix + custom theme)
// ----------------------------------------------------------------------------
// Problem (2026-10-30, user report):
//   Pehle Google, phir DuckDuckGo par bhi "prove you are human" aaya —
//   kyunki search engine ka page WebEngine (browser) me load hota hai,
//   aur engine browser ko bot samajh kar check dikhata hai. v3.1 se
//   DuckDuckGo COREVEX me bilkul nahi hai.
// Fix — engine ka page browser me KABHI load hi nahi hota:
//   * COREVEX khud QNetwork se search source ka lightweight HTML
//     fetch karta hai (Bing -> Brave -> Mojeek, is order me, jab tak
//     ek source kaam na kare). DuckDuckGo is list me nahi hai.
//   * Results ko parse karke apne khud ke COREVEX dark theme HTML me
//     render karta hai (orange accent, #0B0C0F bg, apna search box).
//   * User ko sirf COREVEX ka page dikhta hai, engine ka page nahi —
//     isliye engine ka bot/CAPTCHA page user tak pahunchta hi nahi.
//   * Result par click karne par asli site khulti hai (wo normal site
//     hai, search engine nahi, isliye us par bot check nahi aata).
// Safety:
//   * Bot wale phrases ("captcha", "prove you are human", "verify you
//     are human", "unusual traffic") milne par wo source skip karke
//     agla source try hota hai, user ko bot page kabhi nahi dikhta.
//   * Har source par 12 second timeout hai, phir agla source.
//   * Titles/snippets HTML-escape hote hain, taaki result ka HTML
//     hamara page na tode.
// ============================================================================
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QTimer>

namespace corevex {

struct SearchResult {
    QString title;
    QString url;
    QString snippet;
    // v4.5: dikhane wala domain — Bing ke ck redirect links ke liye
    // host www.bing.com hota hai, par cite se asli domain milta hai.
    // Khali ho to host hi dikhao.
    QString displayDomain;
};

// COREVEX theme me results page, loading page aur error page banata hai.
// Ye sirf HTML string hai — koi network call isme nahi hai.
QString searchResultsHtml(const QString& query, const QList<SearchResult>& results,
                          const QString& sourceName);
QString searchLoadingHtml(const QString& query);
QString searchErrorHtml(const QString& query, const QString& error);

class CustomSearch : public QObject {
    Q_OBJECT
public:
    explicit CustomSearch(QObject* parent = nullptr);
    void search(const QString& query);

signals:
    void resultsReady(const QString& query, const QList<SearchResult>& results,
                      const QString& sourceName);
    void searchFailed(const QString& query, const QString& error);

private slots:
    void onReplyFinished();
    void onTimeout();

private:
    struct Source {
        QString name;
        QString urlTemplate;  // %1 = percent-encoded query
        int parser;           // 0 = DDG Lite, 1 = DDG HTML, 2 = Bing
    };
    void tryNextSource();
    void startSource(const Source& source);
    QList<SearchResult> parseResults(const QString& html, int parser) const;
    static QString stripTags(const QString& html);
    static QString decodeDdgLink(const QString& href);

    // NOTE: Qt classes ko yahan 'class Q...' se forward-declare NAHI karna —
    // wo corevex::QNetworkAccessManager jaisa galat type bana deta hai, aur MSVC
    // 'use of undefined type corevex::QNetworkAccessManager' deta hai.
    // Isliye upar asli Qt headers include kiye hain, aur members global Qt type ke hain.
    QNetworkAccessManager* nam_ = nullptr;
    QNetworkReply* reply_ = nullptr;
    QTimer* timer_ = nullptr;
    QString query_;
    QList<Source> sources_;
    int sourceIndex_ = 0;
};

}  // namespace corevex
