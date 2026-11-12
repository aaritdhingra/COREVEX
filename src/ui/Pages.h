#pragma once
// ============================================================================
// SECTION 8 — REAL SIDEBAR PAGES, DESIGN SYSTEM v2.0 STYLE
// ----------------------------------------------------------------------------
// Design System image ke hisaab se:
//   * Bookmarks: GRID of cards with color thumbnail (list nahi) —
//     design me har bookmark ek thumbnail card hai.
//   * History / Downloads: list rows (design me bhi list hai).
//   * Research: grid cards.
//   * AI Assistant: real chat page (CorevexWindow me, AiClient se).
//   * ADS Lab: visualizations (AdsLabPage me, QPainter charts).
//   * Team Credits: naam + ID, koi role nahi.
// Har page centered, max 800-900px, list/grid poori height fill kare,
// short pages vertically centered — neeche khaali gap nahi.
// ============================================================================
#include <QGridLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QScrollArea>
#include <QWidget>
#include <functional>

namespace corevex {
class Store;

class BookmarksPage : public QWidget {
    Q_OBJECT
public:
    BookmarksPage(Store* store, std::function<void(const QString&)> openUrl, QWidget* parent = nullptr);
    void refresh();
private:
    void applyFilter();
    Store* store_ = nullptr;
    std::function<void(const QString&)> openUrl_;
    QGridLayout* grid_ = nullptr;
    QWidget* gridContainer_ = nullptr;
    QLineEdit* filter_ = nullptr;
};

class HistoryPage : public QWidget {
    Q_OBJECT
public:
    HistoryPage(Store* store, std::function<void(const QString&)> openUrl, QWidget* parent = nullptr);
    void refresh();
private:
    void applyFilter();
    Store* store_ = nullptr;
    std::function<void(const QString&)> openUrl_;
    QListWidget* list_ = nullptr;
    QLineEdit* filter_ = nullptr;
};

class DownloadsPage : public QWidget {
    Q_OBJECT
public:
    explicit DownloadsPage(QWidget* parent = nullptr);
    void refresh();
private:
    void applyFilter();
    QListWidget* list_ = nullptr;
    QLineEdit* filter_ = nullptr;
};

class SearchPage : public QWidget {
    Q_OBJECT
public:
    explicit SearchPage(std::function<void(const QString&)> onSearch, QWidget* parent = nullptr);
private:
    std::function<void(const QString&)> onSearch_;
};

class SettingsPage : public QWidget {
    Q_OBJECT
public:
    explicit SettingsPage(std::function<void()> openAiSettings, QWidget* parent = nullptr);
};

}  // namespace corevex
