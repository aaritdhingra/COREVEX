#ifndef COREVEX_WINDOW_H
#define COREVEX_WINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QTabBar>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QWebEnginePage>
#include <QWebEngineSettings>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QScrollArea>
#include <QLabel>
#include <QFrame>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QShortcut>
#include <QKeySequence>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>
#include <QComboBox>
#include <QCloseEvent>
#include <QProgressBar>
#include <QUrlQuery>
#include <functional>
#include <QPointer>

#include "../ai/AiClient.h"
namespace corevex { class Store; class CustomSearch; }
#include <QListWidget>
#include <QTimer>
#include <QDateTime>
#include <QMenu>
#include <QAction>

struct BookmarkEntry { QString title; QString url; QString favicon; };
struct HistoryEntry { QString title; QString url; QDateTime time; };

class NavItem : public QPushButton {
    Q_OBJECT
public:
    NavItem(const QString &text, const QString &iconPath, QWidget *parent = nullptr);
    void setActive(bool active);
};

class WindowDragFilter : public QObject {
    Q_OBJECT
    QWidget *target;
    bool dragging = false;
    QPoint offset;
public:
    WindowDragFilter(QWidget *t) : QObject(t), target(t) {}
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

// Section 13: WebEngine page jo corevex://search navigation ko pakadta hai.
// acceptNavigationRequest me hi rok do, taaki unknown-scheme error page
// user ko ek second ke liye bhi na dikhe — seedha COREVEX Search chalega.
class CorevexPage : public QWebEnginePage {
    Q_OBJECT
public:
    explicit CorevexPage(QObject* parent = nullptr) : QWebEnginePage(parent) {}
    std::function<void(const QString&)> onCorevexSearch;
    // v4.5: search page se AI panel kholne ke liye (Ask COREvex, Compare, Summarize)
    // task = ask/compare/summarize/deep, q = query. Format: corevex://ai?task=ask&q=...
    std::function<void(const QString&, const QString&)> onCorevexAi;
    // v4.5 LINKS FIX: Bing/Google jaise sites ke result links target=_blank
    // hote hain. Qt me createWindow override na ho to aise links par
    // click karne par kuch nahi hota tha — user ko laga links kaam hi
    // nahi karte. Ab naya window/tab request COREVEX ke naye tab me khulega.
    std::function<QWebEnginePage*()> onNewWindow;
    QWebEnginePage* createWindow(WebWindowType type) override {
        Q_UNUSED(type);
        if (onNewWindow) return onNewWindow();
        return nullptr;
    }
protected:
    bool acceptNavigationRequest(const QUrl& url, NavigationType type, bool isMainFrame) override {
        Q_UNUSED(type); Q_UNUSED(isMainFrame);
        if (url.scheme() == QStringLiteral("corevex") && url.host() == QStringLiteral("search")) {
            QUrlQuery qq(url);
            const QString q = qq.queryItemValue(QStringLiteral("q"));
            if (onCorevexSearch && !q.isEmpty()) onCorevexSearch(q);
            return false;
        }
        if (url.scheme() == QStringLiteral("corevex") && url.host() == QStringLiteral("ai")) {
            QUrlQuery qq(url);
            const QString q = qq.queryItemValue(QStringLiteral("q"));
            const QString task = qq.queryItemValue(QStringLiteral("task"));
            if (onCorevexAi && !q.isEmpty()) onCorevexAi(task, q);
            return false;
        }
        return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
    }
};

class CorevexWindow : public QMainWindow {
    Q_OBJECT
public:
    CorevexWindow(QWidget *parent = nullptr);
    ~CorevexWindow();

    void toggleMaximize();
    void animateMinimize();
    bool isManualMaximized() const { return manualMaximized_; }
    void addNewTab(const QUrl &url = QUrl("about:blank"), bool isPrivate = false);

private slots:
    void handleTabChange(int index);
    void handleUrlEnter();
    void handleNavClick();
    void updateAddressBar(const QUrl &url);
    void updateTabTitle(const QString &title);
    void closeTab(int index);
    void sendAIMessage();
    void goBack();
    void goForward();
    void reloadCurrent();
    void openAiSettings();
    void toggleAIPanel();
    void toggleSidebar();
    void goToBrowser();
    void addBookmark();
    void refreshBookmarkButton();
    void saveSettings();
    void loadSettings();
    void runCorevexSearch(const QString& query);
    // Production browser (v3.0)
    void setupShortcutsAndDownloads();
    void saveSession();
    void zoomIn();
    void zoomOut();
    void zoomReset();
    void duplicateTab();
    void reopenClosedTab();
    void showFindBar();
    void hideFindBar();
    void findNext();
    void findPrev();
    void updateAddressCompleter();
    // AI chatbot v2 (Gemini-style picker)
    void refreshAiModelCombo();
    void loadAiChatHistory();
    void saveAiChatHistory();
    void addAiBubble(const QString& role, const QString& text);
    void scrollAiToBottom();
    void summarizeCurrentPage();
protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void setupUI();
    void buildSidebar();
    void buildTitleBar();
    void buildAddressBar(QVBoxLayout *rightCol);
    void buildAI(QHBoxLayout *bLay);
    QWidget* createNewTabPage();
    QWidget* createAiAssistantPage();
    QWebEngineView* currentView() const;
    QWidget* createSettingsPage();
    QWidget* createADSLabPage();
    QWidget* createMadeByPage();
    QWidget* createSearchPage();
    QWidget* createHistoryPage();
    QWidget* createBookmarksPage();
    QWidget* createDownloadsPage();
    QWidget* createLibraryPage();
    QWidget* wrapPage(QWidget* inner);

    QStackedWidget *mainStack = nullptr;
    QFrame *leftSidebar = nullptr;
    QFrame *titleBar = nullptr;
    QTabBar *tabBar = nullptr;
    QStackedWidget *tabStack = nullptr;
    QLineEdit *addr = nullptr;
    QWidget *aiPanel = nullptr;
    QVBoxLayout *chatLay = nullptr;
    QLineEdit *aiIn = nullptr;
    QScrollArea *aiScroll = nullptr;
    QPushButton *bMax = nullptr;
    QList<NavItem*> navButtons;

    QList<BookmarkEntry> bookmarks;
    QList<HistoryEntry> history;
    corevex::Store* store_ = nullptr;
    corevex::AiClient* aiClient_ = nullptr;
    corevex::CustomSearch* search_ = nullptr;
    QPointer<QWebEngineView> searchView_;
    // v4.3: Google sorry/block track — aakhri search query yaad rakho,
    // taaki Google block kare to usi query ko Bing par khola ja sake.
    QString lastSearchQuery_;
    bool sorryFallbackDone_ = false;
    // AI panel controls
    QComboBox* aiProviderCombo_ = nullptr;
    QComboBox* aiModelCombo_ = nullptr;
    QPushButton* aiStopBtn_ = nullptr;
    QLabel* aiStatusLabel_ = nullptr;
    // Find / download bars
    QWidget* findBar_ = nullptr;
    QLineEdit* findInput_ = nullptr;
    QWidget* downloadBar_ = nullptr;
    QLabel* downloadLabel_ = nullptr;
    QProgressBar* downloadProgress_ = nullptr;
    QPushButton* downloadCancelBtn_ = nullptr;
    class QWebEngineDownloadRequest* currentDownload_ = nullptr;
    // Session / tabs
    QStringList closedTabs_;
    double zoomFactor_ = 1.0;
    class QStringListModel* addrModel_ = nullptr;
    QVector<corevex::AiChatMessage> aiHistory_;
    QPushButton* backBtn_ = nullptr;
    QPushButton* forwardBtn_ = nullptr;
    QPushButton* reloadBtn_ = nullptr;
    QPushButton* bookmarkBtn_ = nullptr;
    QWidget* bookmarksPage_ = nullptr;
    QWidget* historyPage_ = nullptr;
    QWidget* downloadsPage_ = nullptr;
    QRect normalGeometry_;
    bool manualMaximized_ = false;
    bool sidebarVisible_ = true;
    bool aiVisible_ = false;
    bool animatingSidebar_ = false;
    bool animatingAi_ = false;
    QNetworkAccessManager *apiManager = nullptr;
    QSettings *settings = nullptr;
    
    // AI Config
    QString aiProvider, apiKey, baseUrl, aiModel;
};

#endif // COREVEX_WINDOW_H