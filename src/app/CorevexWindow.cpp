// ============================================================================
// COREVEX WINDOW — MAIN SHELL (refactored, modular)
// ----------------------------------------------------------------------------
// Ye file sirf shell hai: sidebar, title bar, address bar, tabs, AI panel.
// Asli kaam alag modules me hai, har module ke upar uska SECTION comment hai:
//   Section 1  utilities/Assets.h        — asset paths (hardcoded path nahi)
//   Section 2  utilities/UrlUtils.h      — URL vs search, encoding
//   Section 3  utilities/ThemeInjector.h — Google search dark theme
//   Section 4  ui/NewTabPage.h           — kaam karta hua new-tab page
//   Section 5  storage/Store.h           — SQLite bookmarks/history
//   Section 6  ai/AiClient.h             — Local Ollama + API chat
//   Section 7  ui/AiSettingsDialog.h     — provider/API settings dialog
//   Section 8  ui/Pages.h                — Bookmarks/History/Downloads/Search/Settings
//   Section 9  ui/AdsLabPage.h           — asli ADS demos
//   Section 10 ads/* + ranking/*         — Trie/Heap/Hash/Graph/AVL/Index/BM25
//   Section 11 search/SearchEngine.h     — Google direct (v4.2), pehle DuckDuckGo/Bing tha
//   Section 13 search/CustomSearch.h     — COREVEX Search: engine page load nahi, custom theme, no bot check
//   Section 12 ui/Animations.h           — sidebar/AI slide, page fade
// ============================================================================
#include "CorevexWindow.h"
#include "../utilities/Assets.h"
#include "../utilities/UrlUtils.h"
#include "../utilities/ThemeInjector.h"
#include "../ui/NewTabPage.h"
#include "../storage/Store.h"
#include "../ui/AiSettingsDialog.h"
#include "../ui/Pages.h"
#include "../ui/AdsLabPage.h"
#include "../ui/Animations.h"
#include "../search/CustomSearch.h"
#include "../search/SearchEngine.h"  // v4.2 fix: searchUrl() isi header me hai, iske bina C2039/C3861 aata tha
#include <QPropertyAnimation>
#include <QSettings>
#include <QApplication>
#include <QGuiApplication>
#include <QClipboard>
#include <QScreen>
#include <QMouseEvent>
#include <QPainter>
#include <QDir>
#include <QStandardPaths>
#include <QScrollArea>
#include <QScrollBar>
#include <QCompleter>
#include <QStringListModel>
#include <QWebEngineDownloadRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QRegularExpression>

NavItem::NavItem(const QString &text, const QString &iconPath, QWidget *parent) : QPushButton(text, parent) {
    setCheckable(true);
    setFixedHeight(40);
    setCursor(Qt::PointingHandCursor);
    setIcon(QIcon(iconPath));
    setIconSize(QSize(20, 20));
    setStyleSheet(R"(
        QPushButton {
            background: transparent; color: #A1A1AA; border: none;
            border-radius: 6px; text-align: left; padding-left: 12px;
            font-size: 13px; font-weight: 500; font-family: 'Segoe UI';
        }
        QPushButton:hover { background: #1B1E24; color: #F4F4F5; }
        QPushButton:checked { 
            background: #1B1E24; color: #F4F4F5; font-weight: 600;
            border-left: 3px solid #FF5A1F; border-top-left-radius: 0px; border-bottom-left-radius: 0px;
            padding-left: 9px;
        }
    )");
}

void NavItem::setActive(bool active) { setChecked(active); }

bool WindowDragFilter::eventFilter(QObject *obj, QEvent *event) {
    // Maximized window ko drag karne se wo ajeeb half-screen position me
    // chala jaata tha — isliye maximized state me drag band hai.
    // Double-click (Chrome jaisa) maximize/restore karta hai.
    if (event->type() == QEvent::MouseButtonDblClick) {
        if (auto* cw = qobject_cast<CorevexWindow*>(target)) { cw->toggleMaximize(); return true; }
        if (auto* w = qobject_cast<QWidget*>(target)) {
            if (w->isMaximized()) w->showNormal(); else w->showMaximized();
        }
        return true;
    }
    if (event->type() == QEvent::MouseButtonPress) {
        auto *me = static_cast<QMouseEvent*>(event);
        if (target->isMaximized()) return false;
        if (auto* cw = qobject_cast<CorevexWindow*>(target)) { if (cw->isManualMaximized()) return false; }
        if (me->button() == Qt::LeftButton) { dragging = true; offset = me->globalPosition().toPoint() - target->pos(); return true; }
    } else if (event->type() == QEvent::MouseMove && dragging) {
        auto *me = static_cast<QMouseEvent*>(event);
        target->move(me->globalPosition().toPoint() - offset); return true;
    } else if (event->type() == QEvent::MouseButtonRelease) { dragging = false; return true; }
    return QObject::eventFilter(obj, event);
}

CorevexWindow::CorevexWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    // v3.3: WA_TranslucentBackground HATA diya — WebEngine ke saath Windows par
    // hover par content flicker/gayab hota tha. Ab window opaque hai.
    resize(1280, 800);
    setMinimumSize(900, 600);
    if (auto* scr = QGuiApplication::primaryScreen()) { const QRect av = scr->availableGeometry(); move(av.x() + (av.width() - width()) / 2, av.y() + (av.height() - height()) / 2); }  // half-screen (960px) par bhi UI toote nahi
    
    apiManager = new QNetworkAccessManager(this);
    settings = new QSettings("COREVEX", "Browser", this);
    // Section 3: Google theme + scrollbar, har tab par apne aap lagega.
    corevex::installCorevexThemeScripts();
    // Section 5: SQLite store (fail ho to app phir bhi chalegi).
    store_ = new corevex::Store();
    store_->open();
    // Section 6: AI client, config Settings se (default: local Ollama).
    aiClient_ = new corevex::AiClient(this);
    // Section 13: COREVEX Search — results fetch + custom theme render.
    search_ = new corevex::CustomSearch(this);
    connect(search_, &corevex::CustomSearch::resultsReady, this, [this](const QString& q, const QList<corevex::SearchResult>& results, const QString& source) {
        if (!searchView_) return;
        searchView_->setHtml(corevex::searchResultsHtml(q, results, source), QUrl());
        const int idx = tabStack ? tabStack->indexOf(searchView_) : -1;
        if (idx >= 0 && idx < tabBar->count()) tabBar->setTabText(idx, q.left(18));
        if (addr) addr->setText(q);
    });
    connect(search_, &corevex::CustomSearch::searchFailed, this, [this](const QString& q, const QString& err) {
        if (!searchView_) return;
        searchView_->setHtml(corevex::searchErrorHtml(q, err), QUrl());
    });
    loadSettings();
    setupUI();
    setupShortcutsAndDownloads();
    // Session restore (v3.3, default ON): pichhli baar khole hue tabs
    // wapas kholo. Agar restore OFF hai ya koi saved tab nahi, to blank tab.
    {
        const bool restore = settings->value(QStringLiteral("session/restore"), true).toBool();
        const QStringList urls = settings->value(QStringLiteral("session/urls")).toStringList();
        const int cur = settings->value(QStringLiteral("session/current"), 0).toInt();
        bool restored = false;
        if (restore) {
            for (const QString& u : urls) {
                const QUrl url(u);
                if (corevex::isSafeWebUrl(url) && url.host() != QStringLiteral("corevex.search") && url.scheme() != QStringLiteral("corevex")) { addNewTab(url); restored = true; }  // v4.2: purani corevex.search tab restore nahi hogi
            }
            if (restored && cur >= 0 && cur < tabBar->count()) { tabBar->setCurrentIndex(cur); tabStack->setCurrentIndex(cur); }
        }
        if (!restored) addNewTab(QUrl("about:blank"));
    }
}

CorevexWindow::~CorevexWindow() {}

void CorevexWindow::setupUI() {
    QWidget *central = new QWidget(this);
    central->setObjectName("centralContainer");
    central->setStyleSheet("QWidget#centralContainer { background: #0B0C0F; border: 1px solid #2A2D35; border-radius: 10px; }");
    setCentralWidget(central);
    
    QHBoxLayout *root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    buildSidebar();
    
    QVBoxLayout *rightCol = new QVBoxLayout();
    rightCol->setContentsMargins(0, 0, 0, 0);
    rightCol->setSpacing(0);
    
    buildTitleBar();
    buildAddressBar(rightCol);
    
    mainStack = new QStackedWidget();

    QWidget *browserPage = new QWidget();
    QHBoxLayout *browserArea = new QHBoxLayout(browserPage);
    browserArea->setContentsMargins(0,0,0,0);
    browserArea->setSpacing(0);

    tabStack = new QStackedWidget();
    browserArea->addWidget(tabStack);
    buildAI(browserArea);

    // Sidebar order se match karna zaroori hai:
    // 0 Browser, 1 Search, 2 AI Assistant, 3 Library, 4 Bookmarks,
    // 5 History, 6 Downloads, 7 ADS Lab, 8 Settings, 9 Made By.
    // Har sidebar page wrapPage() se guzarta hai, taaki uske upar
    // "Back to Browser" + close (X) bar ho — page band karne ka option
    // jo user ne manga tha. Index order wahi rehta hai.
    mainStack->addWidget(browserPage);
    mainStack->addWidget(wrapPage(createSearchPage()));
    mainStack->addWidget(wrapPage(createAiAssistantPage()));
    mainStack->addWidget(wrapPage(createLibraryPage()));
    bookmarksPage_ = createBookmarksPage();
    mainStack->addWidget(wrapPage(bookmarksPage_));
    historyPage_ = createHistoryPage();
    mainStack->addWidget(wrapPage(historyPage_));
    downloadsPage_ = createDownloadsPage();
    mainStack->addWidget(wrapPage(downloadsPage_));
    mainStack->addWidget(wrapPage(createADSLabPage()));
    mainStack->addWidget(wrapPage(createSettingsPage()));
    mainStack->addWidget(wrapPage(createMadeByPage()));           

    rightCol->addWidget(mainStack);
    root->addWidget(leftSidebar);
    root->addLayout(rightCol);
}

void CorevexWindow::buildSidebar() {
    leftSidebar = new QFrame();
    leftSidebar->setMinimumWidth(0);
    leftSidebar->setMaximumWidth(232);
    leftSidebar->setStyleSheet("background: #14161B; border-right: 1px solid #2A2D35;");
    
    QVBoxLayout *lay = new QVBoxLayout(leftSidebar);
    lay->setContentsMargins(12, 16, 12, 16);
    lay->setSpacing(4);

    QLabel *logo = new QLabel();
    logo->setPixmap(QIcon(corevex::assetPath("logos/corevex_wordmark.svg")).pixmap(180, 40));
    logo->setFixedHeight(40);
    logo->setContentsMargins(8, 0, 0, 16);
    lay->addWidget(logo);

    QStringList items = {"Browser", "Search", "AI Assistant", "Research Library", "Bookmarks", "History", "Downloads", "ADS Lab", "Settings", "Made By"};
    QStringList icons = {"nav_browser", "nav_search", "nav_ai", "nav_library", "nav_bookmarks", "nav_history", "nav_downloads", "nav_ads_lab", "nav_settings", "nav_browser"};
    
    for(int i = 0; i < items.size(); ++i) {
        NavItem *nav = new NavItem(items[i], corevex::assetPath("icons/" + icons[i] + ".svg"));
        if(i == 0) nav->setActive(true);
        connect(nav, &QPushButton::clicked, this, &CorevexWindow::handleNavClick);
        navButtons.append(nav);
        lay->addWidget(nav);
        if(i == 6) {
            QFrame *line = new QFrame();
            line->setFrameShape(QFrame::HLine);
            line->setStyleSheet("background-color: #2A2D35; margin: 10px 0px;");
            lay->addWidget(line);
        }
    }
    lay->addStretch();

    QFrame *wm = new QFrame();
    wm->setFixedHeight(90);
    wm->setStyleSheet("QFrame { background: #0B0C0F; border: 1px solid #2A2D35; border-radius: 8px; } QLabel { border: none; }");
    QVBoxLayout *wmLay = new QVBoxLayout(wm);
    wmLay->setContentsMargins(12, 12, 12, 12);
    
    QLabel *wmImg = new QLabel();
    wmImg->setPixmap(QIcon(corevex::assetPath("logos/corevex_wordmark.svg")).pixmap(160, 36));
    
    QLabel *wmTxt = new QLabel("Search. Browse. Think.");
    wmTxt->setStyleSheet("color: #A1A1AA; font-size: 11px; margin-top: 2px;");
    
    wmLay->addWidget(wmImg);
    wmLay->addWidget(wmTxt);
    lay->addWidget(wm);

    // v4.5: "<< Sidebar chhupao" wali text line hata di — user ne bola
    // wo line nahi chahiye. Sidebar toggle ab sirf address bar ke bade
    // button se hoga (neeche, 44x44), isliye sidebar ke neeche koi line nahi.
    QLabel *ver = new QLabel("v4.7  •  Final Build");
    ver->setStyleSheet("color: #A1A1AA; font-size: 11px; margin-left: 8px; margin-top: 8px;");
    lay->addWidget(ver);
}

void CorevexWindow::handleNavClick() {
    NavItem *btn = qobject_cast<NavItem*>(sender());
    if(!btn) return;
    for(int i = 0; i < navButtons.size(); ++i) {
        if(navButtons[i] == btn) {
            navButtons[i]->setActive(true);
            // AI Assistant (index 2) alag page nahi hai — ye right AI panel hai.
            if (i == 2) { mainStack->setCurrentIndex(0); if (aiPanel && !aiVisible_) toggleAIPanel(); }
            else { mainStack->setCurrentIndex(i); if (i != 0) corevex::fadeInPage(mainStack->currentWidget()); }
            // Pages khulne par data refresh ho, taaki list purani na dikhe.
            if (i == 4) { auto* pg = qobject_cast<corevex::BookmarksPage*>(bookmarksPage_); if (pg) pg->refresh(); }
            if (i == 5) { auto* pg = qobject_cast<corevex::HistoryPage*>(historyPage_); if (pg) pg->refresh(); }
            if (i == 6) { auto* pg = qobject_cast<corevex::DownloadsPage*>(downloadsPage_); if (pg) pg->refresh(); }
        } else {
            navButtons[i]->setActive(false);
        }
    }
}void CorevexWindow::buildTitleBar() {
    titleBar = new QFrame();
    titleBar->setFixedHeight(36);
    titleBar->setStyleSheet("background: #0B0C0F;");
    titleBar->installEventFilter(new WindowDragFilter(this));

    QHBoxLayout *lay = new QHBoxLayout(titleBar);
    lay->setContentsMargins(8, 0, 0, 0);
    lay->setSpacing(0);

    tabBar = new QTabBar();
    tabBar->setTabsClosable(false);  // v4.7 X FIX: Qt ka default close button band — uska SVG 28px par scale hokar mota/chunky lag raha tha
    tabBar->setMovable(true);
    tabBar->setStyleSheet(R"(
        QTabBar::tab { background: transparent; color: #A1A1AA; padding: 6px 12px 6px 16px; min-width: 140px; max-width: 220px; font-size: 12px; font-family: 'Segoe UI'; border-right: 1px solid #2A2D35; }
        QTabBar::tab:selected { background: #14161B; color: #F4F4F5; border-top: 2px solid #FF5A1F; border-top-left-radius: 8px; border-top-right-radius: 8px; font-weight: 600; border-right: none; }
    )");
    connect(tabBar, &QTabBar::currentChanged, this, &CorevexWindow::handleTabChange);
    connect(tabBar, &QTabBar::tabCloseRequested, this, &CorevexWindow::closeTab);

    QPushButton *addTab = new QPushButton("+");
    addTab->setFixedSize(28, 28);
    addTab->setStyleSheet("color: #A1A1AA; font-size: 18px; border: none; background: transparent; margin-left: 4px;");
    connect(addTab, &QPushButton::clicked, [this](){ addNewTab(); });

    lay->addWidget(tabBar);
    lay->addWidget(addTab);
    lay->addStretch();

    // Window controls: text glyphs (jo screenshot me ajeeb dikhe the)
    // ki jagah asli SVG icons — min/max/close, Chrome jaisa.
    auto createWinBtn = [&](QString iconPath, QString hoverCol) {
        QPushButton *b = new QPushButton();
        b->setIcon(QIcon(iconPath));
        b->setIconSize(QSize(14, 14));
        b->setFixedSize(46, 36);
        b->setStyleSheet(QString("QPushButton{background:transparent; border:none;} QPushButton:hover{background:%1;}").arg(hoverCol));
        return b;
    };

    QPushButton *bMin = createWinBtn(corevex::assetPath("icons/window_min.svg"), "#1B1E24");
    bMax = createWinBtn(corevex::assetPath("icons/window_max.svg"), "#1B1E24");
    QPushButton *bCls = createWinBtn(corevex::assetPath("icons/window_close.svg"), "#E81123");

    connect(bMin, &QPushButton::clicked, this, &CorevexWindow::animateMinimize);
    connect(bMax, &QPushButton::clicked, this, &CorevexWindow::toggleMaximize);
    connect(bCls, &QPushButton::clicked, this, &CorevexWindow::close);

    lay->addWidget(bMin); lay->addWidget(bMax); lay->addWidget(bCls);
}

void CorevexWindow::buildAddressBar(QVBoxLayout *rightCol) {
    QFrame *bar = new QFrame();
    bar->setFixedHeight(48);
    bar->setStyleSheet("background: #0B0C0F; border-bottom: 1px solid #2A2D35;");
    
    QHBoxLayout *lay = new QHBoxLayout(bar);
    lay->setContentsMargins(10, 0, 10, 0);
    lay->setSpacing(8);
    
    auto createIconBtn = [&](QString iconPath) {
        QPushButton *b = new QPushButton(); b->setIcon(QIcon(iconPath)); b->setFixedSize(36, 36);
        b->setStyleSheet("QPushButton{background:transparent; border:none; border-radius:18px;} QPushButton:hover{background:#1B1E24;}");
        return b;
    };

    // Pehle ye buttons sirf dikhte the, click par kuch nahi karte the.
    // Ab har button kaam karta hai (Section: browser navigation).
    // v4.6 SLEEK: sidebar toggle ka hitbox 44x44 bada hai, par dikhne me
    // sleek hai — transparent background, koi box/border nahi, patli
    // lines wala icon (font 18, bold nahi), hover par halka grey background.
    // Pehle wala 44x44 box + mota icon chunky lag raha tha.
    auto* sideToggle = new QPushButton(QString::fromUtf8("\xE2\x89\xA1"));
    sideToggle->setToolTip("Sidebar dikhao/chhupao");
    sideToggle->setFixedSize(44, 44);
    sideToggle->setStyleSheet("QPushButton{background:transparent; border:none; border-radius:12px; color:#A1A1AA; font-size:18px;} QPushButton:hover{background:#1B1E24; color:#F4F4F5;}");
    connect(sideToggle, &QPushButton::clicked, this, &CorevexWindow::toggleSidebar);
    lay->addWidget(sideToggle);
    backBtn_ = createIconBtn(corevex::assetPath("icons/chrome_back.svg"));
    forwardBtn_ = createIconBtn(corevex::assetPath("icons/chrome_forward.svg"));
    reloadBtn_ = createIconBtn(corevex::assetPath("icons/chrome_reload.svg"));
    connect(backBtn_, &QPushButton::clicked, this, &CorevexWindow::goBack);
    connect(forwardBtn_, &QPushButton::clicked, this, &CorevexWindow::goForward);
    connect(reloadBtn_, &QPushButton::clicked, this, &CorevexWindow::reloadCurrent);
    backBtn_->setToolTip("Back"); forwardBtn_->setToolTip("Forward"); reloadBtn_->setToolTip("Reload");
    lay->addWidget(backBtn_);
    lay->addWidget(forwardBtn_);
    lay->addWidget(reloadBtn_);
    
    // Home button: new-tab page par wapas (pehle home ka koi tareeka nahi tha).
    auto* homeBtn = createIconBtn(corevex::assetPath("icons/chrome_home.svg"));
    homeBtn->setToolTip("Home (new tab page)");
    connect(homeBtn, &QPushButton::clicked, this, [this]{
        if (auto* v = currentView()) v->setHtml(corevex::newTabPageHtml(), QUrl());
        else addNewTab();
    });
    lay->addWidget(homeBtn);

    addr = new QLineEdit();
    addr->setPlaceholderText("Search or enter URL");
    // Address suggestions: bookmarks + history se (production browsers jaisa).
    addrModel_ = new QStringListModel(this);
    auto* completer = new QCompleter(addrModel_, addr);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    addr->setCompleter(completer);
    addr->setStyleSheet("QLineEdit { background: #14161B; color: #F4F4F5; border: 1px solid #2A2D35; border-radius: 18px; padding: 6px 16px; font-size: 13px; } QLineEdit:focus { border: 1px solid #FF5A1F; }");
    connect(addr, &QLineEdit::returnPressed, this, &CorevexWindow::handleUrlEnter);
    lay->addWidget(addr);

    bookmarkBtn_ = createIconBtn(corevex::assetPath("icons/chrome_bookmark.svg"));
    bookmarkBtn_->setToolTip("Bookmark this page");
    connect(bookmarkBtn_, &QPushButton::clicked, this, &CorevexWindow::addBookmark);
    lay->addWidget(bookmarkBtn_);
    auto* dlBtn = createIconBtn(corevex::assetPath("icons/chrome_download.svg"));
    dlBtn->setToolTip("Downloads");
    connect(dlBtn, &QPushButton::clicked, this, [this]{ mainStack->setCurrentIndex(6); });
    auto* stBtn = createIconBtn(corevex::assetPath("icons/chrome_settings.svg"));
    stBtn->setToolTip("Settings");
    connect(stBtn, &QPushButton::clicked, this, [this]{ mainStack->setCurrentIndex(8); });
    lay->addWidget(dlBtn);
    lay->addWidget(stBtn);

    QPushButton *aiBtn = new QPushButton();
    // White sparkle: orange sparkle orange button par dikhta hi nahi tha.
    aiBtn->setIcon(QIcon(corevex::assetPath("icons/ai_sparkle_white.svg")));
    aiBtn->setIconSize(QSize(18, 18));
    aiBtn->setToolTip("COREVEX AI");
    aiBtn->setFixedSize(32, 32);
    aiBtn->setStyleSheet("background: #FF5A1F; border-radius: 16px; border: none;");
    connect(aiBtn, &QPushButton::clicked, this, &CorevexWindow::toggleAIPanel);
    lay->addWidget(aiBtn);

    // Find bar (Ctrl+F): page me text dhoondho, Next/Prev/Close.
    findBar_ = new QWidget();
    findBar_->setVisible(false);
    findBar_->setFixedHeight(40);
    findBar_->setStyleSheet("background:#14161B; border-bottom:1px solid #2A2D35;");
    auto* fl = new QHBoxLayout(findBar_); fl->setContentsMargins(12, 4, 12, 4);
    findInput_ = new QLineEdit(); findInput_->setPlaceholderText("Find in page…");
    findInput_->setStyleSheet("background:#1B1E24; color:#F4F4F5; border:1px solid #2A2D35; border-radius:16px; padding:4px 12px;");
    connect(findInput_, &QLineEdit::returnPressed, this, &CorevexWindow::findNext);
    connect(findInput_, &QLineEdit::textChanged, this, &CorevexWindow::findNext);
    auto* fNext = new QPushButton("Next"); auto* fPrev = new QPushButton("Prev"); auto* fClose = new QPushButton("✕");
    for (auto* b : {fNext, fPrev, fClose}) b->setStyleSheet("color:#F4F4F5; background:transparent; border:1px solid #2A2D35; border-radius:12px; padding:4px 10px;");
    connect(fNext, &QPushButton::clicked, this, &CorevexWindow::findNext);
    connect(fPrev, &QPushButton::clicked, this, &CorevexWindow::findPrev);
    connect(fClose, &QPushButton::clicked, this, &CorevexWindow::hideFindBar);
    fl->addWidget(new QLabel("Find:")); fl->addWidget(findInput_, 1); fl->addWidget(fPrev); fl->addWidget(fNext); fl->addWidget(fClose);

    // Download bar: download progress, cancel, complete par auto-hide.
    downloadBar_ = new QWidget();
    downloadBar_->setVisible(false);
    downloadBar_->setFixedHeight(40);
    downloadBar_->setStyleSheet("background:#14161B; border-bottom:1px solid #2A2D35;");
    auto* dl = new QHBoxLayout(downloadBar_); dl->setContentsMargins(12, 4, 12, 4);
    downloadLabel_ = new QLabel("Downloading…"); downloadLabel_->setStyleSheet("color:#F4F4F5; font-size:13px;");
    downloadProgress_ = new QProgressBar(); downloadProgress_->setRange(0, 100); downloadProgress_->setFixedWidth(220);
    downloadCancelBtn_ = new QPushButton("Cancel");
    downloadCancelBtn_->setStyleSheet("color:#F4F4F5; background:transparent; border:1px solid #2A2D35; border-radius:12px; padding:4px 10px;");
    connect(downloadCancelBtn_, &QPushButton::clicked, this, [this]{ if (currentDownload_) currentDownload_->cancel(); if (downloadBar_) downloadBar_->setVisible(false); });
    dl->addWidget(downloadLabel_, 1); dl->addWidget(downloadProgress_); dl->addWidget(downloadCancelBtn_);

    rightCol->addWidget(titleBar);
    rightCol->addWidget(bar);
    rightCol->addWidget(findBar_);
    rightCol->addWidget(downloadBar_);
}

QWidget* CorevexWindow::createNewTabPage() {
    // Section 4: new-tab HTML ab alag module se aata hai, is file me
    // 100+ lines ka HTML nahi hai. Search form + Quick tiles kaam karte hain.
    QWebEngineView *nt = new QWebEngineView();
    nt->setHtml(corevex::newTabPageHtml(), QUrl());
    return nt;
}void CorevexWindow::buildAI(QHBoxLayout *bLay) {
    aiPanel = new QWidget();
    aiPanel->setMinimumWidth(0);
    aiPanel->setMaximumWidth(380);
    aiPanel->setVisible(false);
    aiPanel->setStyleSheet("background:#14161B; border-left:1px solid #2A2D35;");

    QVBoxLayout *lay = new QVBoxLayout(aiPanel);
    lay->setContentsMargins(14,14,14,14);
    lay->setSpacing(10);

    QHBoxLayout *head = new QHBoxLayout();
    QLabel *lg = new QLabel(); lg->setPixmap(QIcon(corevex::assetPath("icons/ai_logo_mark.svg")).pixmap(20,20));
    QLabel *title = new QLabel("COREVEX AI");
    title->setStyleSheet("color:#F4F4F5; font-size:14px; font-weight:700;");
    head->addWidget(lg); head->addWidget(title);
    auto* aiCloseBtn = new QPushButton();
    aiCloseBtn->setIcon(QIcon(corevex::assetPath("icons/window_close.svg")));
    aiCloseBtn->setIconSize(QSize(12, 12));
    aiCloseBtn->setFixedSize(26, 26);
    aiCloseBtn->setToolTip("Close AI panel");
    aiCloseBtn->setStyleSheet("background:transparent; border:none; border-radius:13px;");
    connect(aiCloseBtn, &QPushButton::clicked, this, &CorevexWindow::toggleAIPanel);
    auto* aiSettingsBtn = new QPushButton("AI Settings");
    aiSettingsBtn->setStyleSheet("color:#FF5A1F; background:transparent; border:1px solid #2A2D35; border-radius:10px; padding:4px 10px;");
    connect(aiSettingsBtn, &QPushButton::clicked, this, &CorevexWindow::openAiSettings);
    head->addStretch();
    head->addWidget(aiSettingsBtn);
    head->addWidget(aiCloseBtn);
    lay->addLayout(head);

    // Gemini-style picker: provider + model, panel se hi badlo.
    // Local aur API dono configure ho to yahan se instant switch hota hai,
    // Settings dialog kholne ki zaroorat nahi.
    auto* picker = new QHBoxLayout();
    aiProviderCombo_ = new QComboBox();
    aiProviderCombo_->addItems({QStringLiteral("local"), QStringLiteral("api"), QStringLiteral("gemini")});
    aiProviderCombo_->setCurrentText(settings ? settings->value(QStringLiteral("ai/provider"), QStringLiteral("local")).toString() : QStringLiteral("local"));
    aiProviderCombo_->setStyleSheet("background:#1B1E24; color:#F4F4F5; border:1px solid #2A2D35; border-radius:10px; padding:4px;");
    aiModelCombo_ = new QComboBox();
    aiModelCombo_->setEditable(true);
    aiModelCombo_->setStyleSheet("background:#1B1E24; color:#F4F4F5; border:1px solid #2A2D35; border-radius:10px; padding:4px;");
    refreshAiModelCombo();
    connect(aiProviderCombo_, &QComboBox::currentTextChanged, this, [this](const QString& prov) {
        if (!settings) return;
        settings->setValue(QStringLiteral("ai/provider"), prov);
        // Provider badalte hi sensible base URL (agar purana provider ka URL hai to).
        if (prov == QStringLiteral("local")) settings->setValue(QStringLiteral("ai/baseUrl"), QStringLiteral("http://127.0.0.1:11434/api/chat"));
        else if (prov == QStringLiteral("gemini")) settings->setValue(QStringLiteral("ai/baseUrl"), QStringLiteral("https://generativelanguage.googleapis.com/v1beta/models"));
        else if (settings->value(QStringLiteral("ai/baseUrl")).toString().contains(QStringLiteral("127.0.0.1")) || settings->value(QStringLiteral("ai/baseUrl")).toString().contains(QStringLiteral("generativelanguage")))
            settings->setValue(QStringLiteral("ai/baseUrl"), QStringLiteral("https://api.openai.com/v1/chat/completions"));
        // Provider badla to model bhi us provider ka pehla preset lo,
        // warna purana provider ka model (jaise llama3 for Gemini) glitch karega.
        settings->setValue(QStringLiteral("ai/model"), corevex::aiModelsForProvider(prov).value(0));
        refreshAiModelCombo();
        loadSettings();
        if (aiStatusLabel_) aiStatusLabel_->setText(QStringLiteral("Provider: ") + prov);
    });
    connect(aiModelCombo_, &QComboBox::currentTextChanged, this, [this](const QString& model) {
        if (!settings || model.isEmpty()) return;
        settings->setValue(QStringLiteral("ai/model"), model);
        loadSettings();
    });
    picker->addWidget(aiProviderCombo_, 1);
    picker->addWidget(aiModelCombo_, 2);
    lay->addLayout(picker);

    // Actions row: New Chat, Summarize Page, Stop, Copy Last (v4.0).
    auto* actions = new QHBoxLayout();
    auto* newChatBtn = new QPushButton("New Chat");
    auto* summarizeBtn = new QPushButton("Summarize Page");
    aiStopBtn_ = new QPushButton("Stop");
    auto* copyLastBtn = new QPushButton("Copy Last");
    aiStopBtn_->setEnabled(false);
    for (auto* b : {newChatBtn, summarizeBtn, aiStopBtn_, copyLastBtn}) b->setStyleSheet("color:#F4F4F5; background:#1B1E24; border:1px solid #2A2D35; border-radius:12px; padding:6px 10px;");
    connect(newChatBtn, &QPushButton::clicked, this, [this]{
        aiHistory_.clear();
        saveAiChatHistory();
        // Chat bubbles saaf karo (stretch ke alawa sab hatao).
        if (chatLay) { while (chatLay->count() > 1) { auto* it = chatLay->takeAt(0); if (it->widget()) it->widget()->deleteLater(); delete it; } }
    });
    connect(summarizeBtn, &QPushButton::clicked, this, &CorevexWindow::summarizeCurrentPage);
    connect(aiStopBtn_, &QPushButton::clicked, this, [this]{ if (aiClient_) aiClient_->cancel(); if (aiStopBtn_) aiStopBtn_->setEnabled(false); });
    connect(copyLastBtn, &QPushButton::clicked, this, [this]{
        for (int i = static_cast<int>(aiHistory_.size()) - 1; i >= 0; --i) {
            if (aiHistory_[i].role == QStringLiteral("assistant")) {
                if (auto* cb = QGuiApplication::clipboard()) cb->setText(aiHistory_[i].content);
                if (aiStatusLabel_) aiStatusLabel_->setText(QStringLiteral("Last answer copied."));
                break;
            }
        }
    });
    actions->addWidget(newChatBtn); actions->addWidget(summarizeBtn); actions->addWidget(aiStopBtn_); actions->addWidget(copyLastBtn);
    lay->addLayout(actions);

    aiStatusLabel_ = new QLabel();
    aiStatusLabel_->setStyleSheet("color:#A1A1AA; font-size:11px;");
    aiStatusLabel_->setText(QStringLiteral("Provider: ") + (aiProviderCombo_ ? aiProviderCombo_->currentText() : QStringLiteral("local")));
    lay->addWidget(aiStatusLabel_);

    aiScroll = new QScrollArea();
    aiScroll->setWidgetResizable(true);
    aiScroll->setStyleSheet("background:transparent; border:none;");
    QWidget *inner = new QWidget();
    chatLay = new QVBoxLayout(inner);
    chatLay->addStretch();
    aiScroll->setWidget(inner);

    aiIn = new QLineEdit();
    aiIn->setPlaceholderText("Ask anything...");
    aiIn->setStyleSheet("background:#1B1E24; color:white; border:1px solid #2A2D35; border-radius:20px; padding:10px 15px;");
    connect(aiIn, &QLineEdit::returnPressed, this, &CorevexWindow::sendAIMessage);

    lay->addWidget(aiScroll);
    lay->addWidget(aiIn);
    bLay->addWidget(aiPanel);
    loadAiChatHistory();
}

void CorevexWindow::refreshAiModelCombo() {
    if (!aiModelCombo_ || !aiProviderCombo_ || !settings) return;
    const QString prov = aiProviderCombo_->currentText();
    const QString saved = settings->value(QStringLiteral("ai/model")).toString();
    aiModelCombo_->blockSignals(true);
    aiModelCombo_->clear();
    aiModelCombo_->addItems(corevex::aiModelsForProvider(prov));
    if (!saved.isEmpty()) aiModelCombo_->setCurrentText(saved);
    aiModelCombo_->blockSignals(false);
}

void CorevexWindow::addAiBubble(const QString& role, const QString& text) {
    if (!chatLay) return;
    auto* msg = new QLabel();
    msg->setWordWrap(true);
    msg->setTextInteractionFlags(Qt::TextSelectableByMouse);
    if (role == QStringLiteral("user")) {
        msg->setText(text.toHtmlEscaped());
        msg->setStyleSheet("background:#FF5A1F; color:white; border-radius:10px; padding:8px; margin-left:40px;");
    } else {
        msg->setTextFormat(Qt::RichText);
        msg->setText(corevex::aiMarkdownToHtml(text));
        msg->setStyleSheet("background:#1B1E24; color:#F4F4F5; border:1px solid #2A2D35; border-radius:10px; padding:8px; margin-right:40px;");
    }
    chatLay->insertWidget(chatLay->count()-1, msg);
    scrollAiToBottom();
}

void CorevexWindow::scrollAiToBottom() {
    if (!aiScroll) return;
    QTimer::singleShot(50, this, [this] {
        if (aiScroll && aiScroll->verticalScrollBar()) aiScroll->verticalScrollBar()->setValue(aiScroll->verticalScrollBar()->maximum());
    });
}

void CorevexWindow::saveAiChatHistory() {
    if (!settings) return;
    QJsonArray arr;
    for (const auto& m : aiHistory_) { QJsonObject o; o[QStringLiteral("role")] = m.role; o[QStringLiteral("content")] = m.content; arr.append(o); }
    settings->setValue(QStringLiteral("ai/chatHistory"), QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact)));
}

void CorevexWindow::loadAiChatHistory() {
    if (!settings || !chatLay) return;
    aiHistory_.clear();
    const QJsonDocument doc = QJsonDocument::fromJson(settings->value(QStringLiteral("ai/chatHistory")).toString().toUtf8());
    if (!doc.isArray()) return;
    for (const auto& v : doc.array()) {
        const QJsonObject o = v.toObject();
        const QString role = o.value(QStringLiteral("role")).toString();
        const QString content = o.value(QStringLiteral("content")).toString();
        if (content.isEmpty()) continue;
        aiHistory_.append({role, content});
        addAiBubble(role, content);
    }
    if (aiHistory_.size() > 40) aiHistory_ = aiHistory_.mid(aiHistory_.size() - 40);
}

void CorevexWindow::summarizeCurrentPage() {
    auto* view = currentView();
    if (!view || !aiClient_) return;
    if (!aiVisible_) toggleAIPanel();
    addAiBubble(QStringLiteral("user"), QStringLiteral("Summarize this page"));
    auto* aMsg = new QLabel("Summarizing page…");
    aMsg->setWordWrap(true);
    aMsg->setTextInteractionFlags(Qt::TextSelectableByMouse);
    aMsg->setStyleSheet("background:#1B1E24; color:#F4F4F5; border:1px solid #2A2D35; border-radius:10px; padding:8px; margin-right:40px;");
    chatLay->insertWidget(chatLay->count()-1, aMsg);
    if (aiStopBtn_) aiStopBtn_->setEnabled(true);
    loadSettings();
    view->page()->toHtml([this, aMsg](const QString& html) {
        QString text = html;
        text.remove(QRegularExpression(QStringLiteral("<script[^>]*>.*?</script>"), QRegularExpression::DotMatchesEverythingOption));
        text.remove(QRegularExpression(QStringLiteral("<style[^>]*>.*?</style>"), QRegularExpression::DotMatchesEverythingOption));
        text.remove(QRegularExpression(QStringLiteral("<[^>]+>")));
        text = text.simplified().left(8000);
        const QString prompt = QStringLiteral("Summarize this page in simple, short points (max 8 points). Page text:") + QStringLiteral("\n\n") + text;
        aiClient_->chat(aiHistory_, prompt,
            [this, aMsg](const QString& reply) {
                aMsg->setTextFormat(Qt::RichText);
                aMsg->setText(corevex::aiMarkdownToHtml(reply));
                if (aiStopBtn_) aiStopBtn_->setEnabled(false);
                aiHistory_.append({QStringLiteral("user"), QStringLiteral("Summarize this page")});
                aiHistory_.append({QStringLiteral("assistant"), reply});
                if (aiHistory_.size() > 40) aiHistory_.removeFirst();
                saveAiChatHistory();
            },
            [this, aMsg](const QString& err) {
                aMsg->setText(QStringLiteral("AI Error: ") + err);
                if (aiStopBtn_) aiStopBtn_->setEnabled(false);
            });
    });
}

// --- Helper: current tab ka WebEngineView (agar ho to) -------------------
QWebEngineView* CorevexWindow::currentView() const {
    return qobject_cast<QWebEngineView*>(tabStack ? tabStack->currentWidget() : nullptr);
}
void CorevexWindow::goBack() { if (auto* v = currentView()) v->back(); }
void CorevexWindow::goForward() { if (auto* v = currentView()) v->forward(); }
void CorevexWindow::reloadCurrent() { if (auto* v = currentView()) v->reload(); }
void CorevexWindow::openAiSettings() {
    corevex::AiSettingsDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) loadSettings();
}

void CorevexWindow::addNewTab(const QUrl &url, bool isPrivate) {
    Q_UNUSED(isPrivate);
    // Har tab ek hi tarah ka WebEngineView hai — new-tab page bhi wahi
    // view hai, sirf uska HTML alag hai. Isse title/url/history signals
    // har tab par ek jaise kaam karte hain.
    QWebEngineView *view = new QWebEngineView();
    auto* page = new CorevexPage(view);
    page->setBackgroundColor(QColor(QStringLiteral("#0B0C0F")));
    view->setPage(page);
    page->onCorevexSearch = [this](const QString& q) { runCorevexSearch(q); };
    page->onNewWindow = [this]() -> QWebEnginePage* {
        addNewTab();
        auto* v = currentView();
        return v ? v->page() : nullptr;
    };
    page->onCorevexAi = [this](const QString& task, const QString& q) {
        mainStack->setCurrentIndex(0);
        if (aiPanel && !aiVisible_) toggleAIPanel();
        QString prompt;
        if (task == QStringLiteral("compare")) prompt = QStringLiteral("Compare top sources for: ") + q;
        else if (task == QStringLiteral("summarize")) prompt = QStringLiteral("Summarize search results for: ") + q;
        else if (task == QStringLiteral("deep")) prompt = QStringLiteral("Deep research: ") + q;
        else prompt = QStringLiteral("Explain: ") + q;
        if (aiIn) { aiIn->setText(prompt); aiIn->setFocus(); }
    };
    if (url.toString() == "about:blank") {
        view->setHtml(corevex::newTabPageHtml(), QUrl());
    } else {
        view->setUrl(url);
    }
    connect(view, &QWebEngineView::titleChanged, this, &CorevexWindow::updateTabTitle);
    connect(view, &QWebEngineView::urlChanged, this, &CorevexWindow::updateAddressBar);
    connect(view, &QWebEngineView::loadFinished, this, [this, view](bool ok) {
        saveSession();
        updateAddressCompleter();
        refreshBookmarkButton();
        // v4.3 GOOGLE SORRY FIX: Google ne search ko bot samajh kar
        // /sorry/index par bhej diya (screenshot me wahi URL tha).
        // Pehle COREVEX isko load-failure samajh kar apna "can't be
        // reached" page dikha deta tha, isliye user captcha tak pahunch
        // hi nahi pata tha. Ab: sorry aate hi, ek baar, usi query ko
        // Bing par kholo — search toote nahi, results mil jayein.
        // Bing bhi fail ho to generic error page nahi, seedha wajah
        // batane wala page dikhao (Retry = Bing, dobara Google try).
        if (view->url().host().endsWith(QStringLiteral("google.com")) &&
            view->url().path().startsWith(QStringLiteral("/sorry"))) {
            // New-tab form se aaya search handleUrlEnter se nahi guzarta,
            // isliye aakhri query stale ho sakti hai — sorry URL ke
            // continue parameter me asli search URL chhupa hota hai.
            { QUrlQuery sq(view->url()); const QString cont = sq.queryItemValue(QStringLiteral("continue"));
              if (!cont.isEmpty()) { const QUrl contUrl(cont); QUrlQuery cq(contUrl); const QString cqQ = cq.queryItemValue(QStringLiteral("q"));  // v4.4 fix: QUrlQuery cq(QUrl(cont)) MSVC me function declaration samajh liya jata tha (most-vexing-parse), isliye C2228 aata tha
                if (!cqQ.isEmpty()) lastSearchQuery_ = cqQ; } }
            if (!sorryFallbackDone_ && !lastSearchQuery_.isEmpty()) {
                sorryFallbackDone_ = true;
                view->setUrl(corevex::searchUrlForEngine(lastSearchQuery_, QStringLiteral("bing")));
                return;
            }
            const QString bingUrl = corevex::searchUrlForEngine(lastSearchQuery_, QStringLiteral("bing")).toString().toHtmlEscaped();
            const QString googleUrl = corevex::searchUrlForEngine(lastSearchQuery_, QStringLiteral("google")).toString().toHtmlEscaped();
            view->setHtml(QStringLiteral("<html><head><meta charset='utf-8'><style>body{margin:0;background:#0B0C0F;color:#F4F4F5;font-family:'Segoe UI';} .w{max-width:640px;margin:80px auto;padding:24px;} .c{background:#14161B;border:1px solid #2A2D35;border-radius:12px;padding:28px;line-height:1.7;} a{color:#FF5A1F;font-weight:700;}</style></head><body><div class='w'><div class='c'><b style='font-size:20px;'>Google ne search block kar diya</b><br/><br/>Google tumhare browser ko bot samajh raha hai, isliye usne sorry page par bhej diya — ye COREVEX ki galti nahi, Google ka block hai. Isliye search yahan rukna nahi chahiye:<br/><br/><a href='%1'>Bing par '%2' search karo (kaam karega)</a><br/><br/><a href='%3'>Google dobara try karo</a> (kuch der baad ya Chrome me captcha solve karke)</div></div></body></html>").arg(bingUrl, lastSearchQuery_.toHtmlEscaped(), googleUrl), QUrl());
            return;
        }
        if (!ok) {
            // Production error page: WebEngine ka default error nahi, COREVEX theme me retry.
            if (corevex::isSafeWebUrl(view->url()) && view->url().host() != QStringLiteral("corevex.search") && view->url().scheme() != QStringLiteral("corevex")) {
                const QString u = view->url().toString().toHtmlEscaped();
                view->setHtml(QStringLiteral("<html><head><meta charset='utf-8'><style>body{margin:0;background:#0B0C0F;color:#F4F4F5;font-family:'Segoe UI';} .w{max-width:640px;margin:80px auto;padding:24px;} .c{background:#14161B;border:1px solid #2A2D35;border-radius:12px;padding:28px;line-height:1.7;} a{color:#FF5A1F;font-weight:700;}</style></head><body><div class='w'><div class='c'><b style='font-size:20px;'>This site can't be reached</b><br/><span style='color:#A1A1AA;'>%1</span><br/><br/>Internet check karo, spelling check karo, ya <a href='%1'>Retry</a> karo.<br/>Agar site ka naam ek word tha (jaise 'youtube'), to wo search nahi, site hai — address me .com lagao.</div></div></body></html>").arg(u), QUrl());
            }
            return;
        }
        if (!store_ || !store_->isOpen()) return;
        if (corevex::isSafeWebUrl(view->url()) && !view->title().isEmpty() && view->url().host() != QStringLiteral("corevex.search"))
            store_->addHistory(view->title(), view->url().toString());
    });
    tabStack->addWidget(view);
    int idx = tabBar->addTab("New Tab");
    // v4.7 X FIX (sirf ye): Qt ke SVG close button ki jagah apna sleek
    // text X button. Hitbox 28x28 bada hai, par dikhne me patla hai —
    // font 14, normal weight, grey, SVG scale hokar mota nahi hoga.
    // Tab ke right side me alag button hai, isliye text se takrayega
    // nahi aur edge se chipkega nahi.
    {
        auto* xBtn = new QPushButton(QString::fromUtf8("\xC3\x97"));
        xBtn->setToolTip("Tab band karo");
        xBtn->setFixedSize(28, 28);
        xBtn->setStyleSheet("QPushButton{background:transparent; border:none; border-radius:14px; color:#A1A1AA; font-size:14px;} QPushButton:hover{background:#F02D24; color:#FFFFFF;}");
        connect(xBtn, &QPushButton::clicked, this, [this, xBtn]() {
            for (int i = 0; i < tabBar->count(); ++i)
                if (tabBar->tabButton(i, QTabBar::RightSide) == xBtn) { closeTab(i); break; }
        });
        tabBar->setTabButton(idx, QTabBar::RightSide, xBtn);
    }
    tabBar->setCurrentIndex(idx);
    tabStack->setCurrentIndex(idx);
    if (mainStack) mainStack->setCurrentIndex(0);
    refreshBookmarkButton();
    // v4.1 CHROME-STYLE FOCUS: naya blank tab khulte hi address bar par cursor
    // aa jaye, taaki user bina click kiye seedha type kar sake (typing mode ON).
    if (url.toString() == "about:blank" && addr) {
        QTimer::singleShot(50, this, [this]{ if (addr) { addr->setFocus(); addr->selectAll(); } });
    }
}

void CorevexWindow::sendAIMessage() {
    QString txt = aiIn->text().trimmed();
    if (txt.isEmpty() || !aiClient_) return;
    addAiBubble(QStringLiteral("user"), txt);
    aiIn->clear();
    auto* aMsg = new QLabel("Thinking…");
    aMsg->setWordWrap(true);
    aMsg->setTextInteractionFlags(Qt::TextSelectableByMouse);
    aMsg->setStyleSheet("background:#1B1E24; color:#F4F4F5; border:1px solid #2A2D35; border-radius:10px; padding:8px; margin-right:40px;");
    chatLay->insertWidget(chatLay->count()-1, aMsg);
    if (aiStopBtn_) aiStopBtn_->setEnabled(true);
    if (aiStatusLabel_) aiStatusLabel_->setText(QStringLiteral("Thinking… (") + aiProvider + QStringLiteral(" / ") + aiModel + QStringLiteral(")"));
    loadSettings();
    aiClient_->chat(aiHistory_, txt,
        [this, aMsg, txt](const QString& reply) {
            aMsg->setTextFormat(Qt::RichText);
            aMsg->setText(corevex::aiMarkdownToHtml(reply));
            scrollAiToBottom();
            if (aiStopBtn_) aiStopBtn_->setEnabled(false);
            if (aiStatusLabel_) aiStatusLabel_->setText(QStringLiteral("Provider: ") + aiProvider + QStringLiteral(" / ") + aiModel);
            aiHistory_.append({QStringLiteral("user"), txt});
            aiHistory_.append({QStringLiteral("assistant"), reply});
            if (aiHistory_.size() > 40) aiHistory_.removeFirst();
            saveAiChatHistory();
        },
        [this, aMsg](const QString& err) {
            aMsg->setText(err.contains(QStringLiteral("Stopped")) ? QStringLiteral("Stopped.") : QStringLiteral("AI Error: ") + err);
            if (aiStopBtn_) aiStopBtn_->setEnabled(false);
        });
}

void CorevexWindow::toggleAIPanel() {
    if (!aiPanel || animatingAi_) return;
    animatingAi_ = true;
    if (!aiVisible_) {
        aiPanel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        aiPanel->setVisible(true);
        aiPanel->setMaximumWidth(0);
        auto* anim = new QPropertyAnimation(aiPanel, "maximumWidth", this);
        anim->setDuration(250); anim->setStartValue(0); anim->setEndValue(360);
        anim->setEasingCurve(QEasingCurve::InOutCubic);
        connect(anim, &QPropertyAnimation::finished, this, [this]{ animatingAi_ = false; aiVisible_ = true; });
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        aiPanel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
        auto* anim = new QPropertyAnimation(aiPanel, "maximumWidth", this);
        anim->setDuration(250); anim->setStartValue(aiPanel->maximumWidth()); anim->setEndValue(0);
        anim->setEasingCurve(QEasingCurve::InOutCubic);
        connect(anim, &QPropertyAnimation::finished, this, [this]{ aiPanel->setVisible(false); animatingAi_ = false; aiVisible_ = false; });
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void CorevexWindow::toggleSidebar() {
    if (!leftSidebar || animatingSidebar_) return;
    animatingSidebar_ = true;
    if (sidebarVisible_) {
        leftSidebar->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
        auto* anim = new QPropertyAnimation(leftSidebar, "maximumWidth", this);
        anim->setDuration(250); anim->setStartValue(232); anim->setEndValue(0);
        anim->setEasingCurve(QEasingCurve::InOutCubic);
        connect(anim, &QPropertyAnimation::finished, this, [this]{ leftSidebar->setVisible(false); animatingSidebar_ = false; sidebarVisible_ = false; });
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        leftSidebar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        leftSidebar->setVisible(true);
        leftSidebar->setMaximumWidth(0);
        auto* anim = new QPropertyAnimation(leftSidebar, "maximumWidth", this);
        anim->setDuration(250); anim->setStartValue(0); anim->setEndValue(232);
        anim->setEasingCurve(QEasingCurve::InOutCubic);
        connect(anim, &QPropertyAnimation::finished, this, [this]{ animatingSidebar_ = false; sidebarVisible_ = true; });
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void CorevexWindow::goToBrowser() {
    mainStack->setCurrentIndex(0);
    for (auto* n : navButtons) n->setActive(n == navButtons.value(0));
}

QWidget* CorevexWindow::wrapPage(QWidget* inner) {
    auto* w = new QWidget();
    auto* lay = new QVBoxLayout(w);
    lay->setContentsMargins(0, 0, 0, 0); lay->setSpacing(0);
    auto* bar = new QWidget();
    bar->setFixedHeight(40);
    bar->setStyleSheet("background:#0B0C0F; border-bottom:1px solid #2A2D35;");
    auto* h = new QHBoxLayout(bar); h->setContentsMargins(16, 0, 12, 0);
    auto* back = new QPushButton(QString::fromUtf8("\xE2\x86\x90  Browser"));
    back->setStyleSheet("background:transparent; color:#FF5A1F; border:none; font-weight:600;");
    connect(back, &QPushButton::clicked, this, &CorevexWindow::goToBrowser);
    auto* closeBtn = new QPushButton(QString::fromUtf8("\xE2\x9C\x95"));
    closeBtn->setFixedSize(28, 28);
    closeBtn->setToolTip("Page band karo, Browser par wapas jao");
    closeBtn->setStyleSheet("background:transparent; color:#A1A1AA; border:none; font-size:14px;");
    connect(closeBtn, &QPushButton::clicked, this, &CorevexWindow::goToBrowser);
    h->addWidget(back); h->addStretch(); h->addWidget(closeBtn);
    lay->addWidget(bar); lay->addWidget(inner, 1);
    return w;
}

void CorevexWindow::handleUrlEnter() {
    QString input = addr->text().trimmed();
    if(input.isEmpty()) return;
    // Section 2: URL vs search + sahi encoding. Purana code view ko
    // replace karke history mita deta tha; ab current view me hi load
    // hota hai, taaki Back/Forward kaam kare.
    const QUrl url = corevex::resolveAddressInput(input);
    // v4.5: Bing/Google/Brave/Mojeek ke search URL par seedha engine page
    // nahi, COREVEX ka apna search page (last image jaisa theme) kholo.
    // Sirf tab jab user ne Settings me Google direct chuna ho, tab Google page.
    if (corevex::searchEngineId() != QStringLiteral("google") &&
        (url.host().contains(QStringLiteral("bing")) || url.host().contains(QStringLiteral("brave")) ||
         url.host().contains(QStringLiteral("mojeek")) || url.host().contains(QStringLiteral("google")))) {
        QUrlQuery qq2(url);
        const QString sq2 = qq2.queryItemValue(QStringLiteral("q"));
        if (!sq2.isEmpty() && url.path().contains(QStringLiteral("search"))) {
            runCorevexSearch(sq2);
            if (addr) addr->clearFocus();
            if (auto* view = currentView()) view->setFocus();
            return;
        }
    }
    { QUrlQuery qq(url); const QString sq = qq.queryItemValue(QStringLiteral("q"));
      if (!sq.isEmpty() && (url.host().contains(QStringLiteral("google")) || url.host().contains(QStringLiteral("bing")) ||
          url.host().contains(QStringLiteral("brave")) || url.host().contains(QStringLiteral("mojeek")))) {
          lastSearchQuery_ = sq; sorryFallbackDone_ = false; } }
    if (url.scheme() == QStringLiteral("corevex")) {
        QUrlQuery qq(url);
        runCorevexSearch(qq.queryItemValue(QStringLiteral("q")));
        if (addr) addr->clearFocus();
        if (auto* view = currentView()) view->setFocus();
        return;
    }
    if (auto* view = currentView()) view->setUrl(url);
    else addNewTab(url);
    // v4.1: Enter ke baad address bar ka focus hat jaye (typing mode OFF),
    // taaki page par shortcuts / scroll kaam kare, aur address bar focused na rahe.
    if (addr) addr->clearFocus();
    if (auto* view = currentView()) view->setFocus();
}

void CorevexWindow::runCorevexSearch(const QString& query) {
    // v4.5: search ab COREVEX ke apne search page par hota hai (last image
    // jaisa theme), engine ka page direct nahi khulta — isliye UI broken
    // nahi lagega aur links bhi kaam karenge (createWindow fix).
    // Sirf Google direct chuna ho to purana Google page (sorry fallback ke saath).
    const QString q = query.trimmed();
    if (q.isEmpty()) return;
    lastSearchQuery_ = q;
    sorryFallbackDone_ = false;
    mainStack->setCurrentIndex(0);
    QWebEngineView* view = currentView();
    if (!view) { addNewTab(); view = currentView(); }
    if (!view) return;
    searchView_ = view;
    if (addr) addr->setText(q);
    if (corevex::searchEngineId() == QStringLiteral("google")) {
        view->setUrl(corevex::searchUrlForEngine(q, QStringLiteral("google")));
    } else {
        view->setHtml(corevex::searchLoadingHtml(q), QUrl());
        if (search_) search_->search(q);
    }
    if (addr) addr->clearFocus();
    if (view) view->setFocus();
}

void CorevexWindow::setupShortcutsAndDownloads() {
    auto addSc = [this](const QKeySequence& seq, auto fn) {
        auto* sc = new QShortcut(seq, this);
        connect(sc, &QShortcut::activated, this, fn);
    };
    addSc(QKeySequence::AddTab, [this]{ addNewTab(); });
    addSc(QKeySequence::Close, [this]{ if (tabBar) closeTab(tabBar->currentIndex()); });
    addSc(QKeySequence::Refresh, [this]{ reloadCurrent(); });
    addSc(QKeySequence(Qt::CTRL | Qt::Key_L), [this]{ if (addr) { addr->setFocus(); addr->selectAll(); } });
    addSc(QKeySequence::Find, [this]{ showFindBar(); });
    addSc(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_T), [this]{ reopenClosedTab(); });
    addSc(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_D), [this]{ duplicateTab(); });
    addSc(QKeySequence(Qt::CTRL | Qt::Key_Plus), [this]{ zoomIn(); });
    addSc(QKeySequence(Qt::CTRL | Qt::Key_Minus), [this]{ zoomOut(); });
    addSc(QKeySequence(Qt::CTRL | Qt::Key_0), [this]{ zoomReset(); });
    addSc(QKeySequence::NextChild, [this]{ if (tabBar && tabBar->count() > 1) tabBar->setCurrentIndex((tabBar->currentIndex() + 1) % tabBar->count()); });
    addSc(QKeySequence::PreviousChild, [this]{ if (tabBar && tabBar->count() > 1) tabBar->setCurrentIndex((tabBar->currentIndex() - 1 + tabBar->count()) % tabBar->count()); });
    // v4.0: Ctrl+1..9 se seedha us number ka tab kholo (Chrome jaisa).
    for (int i = 1; i <= 9; ++i) {
        addSc(QKeySequence(Qt::CTRL | static_cast<Qt::Key>(Qt::Key_0 + i)), [this, i]{ if (tabBar && i <= tabBar->count()) tabBar->setCurrentIndex(i - 1); });
    }
    // Downloads: WebEngine ka download request yahan pakdo, warna download
    // chupchaap folder me jaata tha aur user ko progress hi nahi dikhta tha.
    auto* profile = QWebEngineProfile::defaultProfile();
    if (profile) {
        connect(profile, &QWebEngineProfile::downloadRequested, this, [this](QWebEngineDownloadRequest* dl) {
            dl->setDownloadDirectory(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
            dl->accept();
            currentDownload_ = dl;
            if (downloadBar_) downloadBar_->setVisible(true);
            if (downloadLabel_) downloadLabel_->setText(QStringLiteral("Downloading: ") + dl->downloadFileName());
            connect(dl, &QWebEngineDownloadRequest::receivedBytesChanged, this, [this, dl] {
                if (dl->totalBytes() > 0 && downloadProgress_) downloadProgress_->setValue(static_cast<int>(dl->receivedBytes() * 100 / dl->totalBytes()));
            });
            connect(dl, &QWebEngineDownloadRequest::stateChanged, this, [this, dl](QWebEngineDownloadRequest::DownloadState state) {
                if (state == QWebEngineDownloadRequest::DownloadCompleted) {
                    if (downloadLabel_) downloadLabel_->setText(QStringLiteral("Download complete: ") + dl->downloadFileName());
                    if (downloadProgress_) downloadProgress_->setValue(100);
                    if (downloadsPage_) { auto* pg = qobject_cast<corevex::DownloadsPage*>(downloadsPage_); if (pg) pg->refresh(); }
                    QTimer::singleShot(4000, this, [this]{ if (downloadBar_) downloadBar_->setVisible(false); });
                    currentDownload_ = nullptr;
                } else if (state == QWebEngineDownloadRequest::DownloadCancelled || state == QWebEngineDownloadRequest::DownloadInterrupted) {
                    if (downloadLabel_) downloadLabel_->setText(QStringLiteral("Download stopped: ") + dl->downloadFileName());
                    currentDownload_ = nullptr;
                }
            });
        });
    }
}

void CorevexWindow::saveSession() {
    if (!settings || !tabStack) return;
    QStringList urls;
    for (int i = 0; i < tabStack->count(); ++i) {
        auto* v = qobject_cast<QWebEngineView*>(tabStack->widget(i));
        if (v && corevex::isSafeWebUrl(v->url()) && v->url().host() != QStringLiteral("corevex.search") && v->url().scheme() != QStringLiteral("corevex")) urls.append(v->url().toString());  // v4.2: corevex.search kabhi save nahi, warna restart par "site can't be reached" aata
    }
    settings->setValue(QStringLiteral("session/urls"), urls);
    settings->setValue(QStringLiteral("session/current"), tabBar ? tabBar->currentIndex() : 0);
}

void CorevexWindow::closeEvent(QCloseEvent* event) {
    saveSession();
    saveAiChatHistory();
    QMainWindow::closeEvent(event);
}

void CorevexWindow::zoomIn() { zoomFactor_ = qMin(3.0, zoomFactor_ + 0.1); if (auto* v = currentView()) v->setZoomFactor(zoomFactor_); }
void CorevexWindow::zoomOut() { zoomFactor_ = qMax(0.25, zoomFactor_ - 0.1); if (auto* v = currentView()) v->setZoomFactor(zoomFactor_); }
void CorevexWindow::zoomReset() { zoomFactor_ = 1.0; if (auto* v = currentView()) v->setZoomFactor(zoomFactor_); }
void CorevexWindow::duplicateTab() { if (auto* v = currentView()) { if (corevex::isSafeWebUrl(v->url())) addNewTab(v->url()); else addNewTab(); } }
void CorevexWindow::reopenClosedTab() { if (!closedTabs_.isEmpty()) addNewTab(QUrl(closedTabs_.takeLast())); }
void CorevexWindow::showFindBar() { if (findBar_) { findBar_->setVisible(true); if (findInput_) findInput_->setFocus(); } }
void CorevexWindow::hideFindBar() { if (findBar_) findBar_->setVisible(false); if (auto* v = currentView()) v->page()->findText(QString()); }
void CorevexWindow::findNext() { if (auto* v = currentView(); v && findInput_) v->page()->findText(findInput_->text()); }
void CorevexWindow::findPrev() { if (auto* v = currentView(); v && findInput_) v->page()->findText(findInput_->text(), QWebEnginePage::FindBackward); }
void CorevexWindow::updateAddressCompleter() {
    if (!addrModel_ || !store_ || !store_->isOpen()) return;
    QStringList items;
    for (const auto& b : store_->bookmarks()) { items.append(b.url); items.append(b.title); }
    for (const auto& h : store_->history(200)) { items.append(h.url); items.append(h.title); }
    items.removeDuplicates();
    addrModel_->setStringList(items);
}

void CorevexWindow::handleTabChange(int index) {
    if(index == -1) return;
    saveSession();
    updateAddressCompleter();
    tabStack->setCurrentIndex(index);
    refreshBookmarkButton();
    QWidget *w = tabStack->widget(index);
    if(auto *v = qobject_cast<QWebEngineView*>(w)) {
        updateAddressBar(v->url());
    } else {
        addr->setText("");
    }
}

void CorevexWindow::updateTabTitle(const QString &title) {
    // Sender wala tab hi update ho, current tab nahi — warna background
    // tab load hone par galat tab ka title badal jaata.
    auto* view = qobject_cast<QWebEngineView*>(sender());
    const int idx = view ? tabStack->indexOf(view) : tabStack->currentIndex();
    if(idx >= 0 && idx < tabBar->count()) tabBar->setTabText(idx, title.isEmpty() ? QStringLiteral("New Tab") : title.left(18));
}

// Data URI / about:blank address bar me garbage na dikhaye, isliye blank.
// Sirf current tab ka URL address bar me dikhe, background tab ka nahi.
void CorevexWindow::updateAddressBar(const QUrl &url) {
    auto* view = qobject_cast<QWebEngineView*>(sender());
    if (view && view != currentView()) return;
    if(url.scheme() == "data" || url.toString() == "about:blank") {
        addr->setText("");
    } else {
        addr->setText(url.toString());
    }
    refreshBookmarkButton();
}

void CorevexWindow::closeTab(int index) {
    if(tabBar->count() <= 1) return;
    if (auto* v = qobject_cast<QWebEngineView*>(tabStack->widget(index)); v && corevex::isSafeWebUrl(v->url())) {
        closedTabs_.append(v->url().toString());
        if (closedTabs_.size() > 10) closedTabs_.removeFirst();
    }
    QWidget *w = tabStack->widget(index);
    saveSession();
    tabStack->removeWidget(w);
    tabBar->removeTab(index);
    w->deleteLater();
}

void CorevexWindow::toggleMaximize() {
    // v4.1 TASKBAR FIX: showMaximized() frameless window me Windows taskbar ko
    // dhak deta tha (window full screen geometry le leti thi). Isliye manual
    // maximize: geometry = screen ka availableGeometry (taskbar excluded),
    // 200ms geometry animation ke saath, taaki maximize/minimize animation bhi dikhe.
    if (manualMaximized_ || isMaximized()) {
        manualMaximized_ = false;
        showNormal();
        if (normalGeometry_.isValid()) setGeometry(normalGeometry_);
        if (centralWidget()) centralWidget()->setStyleSheet("QWidget#centralContainer { background: #0B0C0F; border: 1px solid #2A2D35; border-radius: 10px; }");
    } else {
        normalGeometry_ = geometry();
        manualMaximized_ = true;
        if (centralWidget()) centralWidget()->setStyleSheet("QWidget#centralContainer { background: #0B0C0F; border: none; border-radius: 0px; }");
        if (auto* scr = QGuiApplication::primaryScreen()) {
            const QRect avail = scr->availableGeometry();
            auto* anim = new QPropertyAnimation(this, "geometry", this);
            anim->setDuration(200); anim->setStartValue(geometry()); anim->setEndValue(avail);
            anim->setEasingCurve(QEasingCurve::OutCubic);
            connect(anim, &QPropertyAnimation::finished, this, [this, avail]{ setGeometry(avail); });
            anim->start(QAbstractAnimation::DeleteWhenStopped);
        } else {
            showMaximized();
        }
    }
}

void CorevexWindow::animateMinimize() {
    // v4.1: Frameless window par Windows ki default minimize animation nahi
    // aati thi, isliye khud fade animation: opacity 1 -> 0 (180ms), phir minimize.
    if (isMinimized()) return;
    auto* anim = new QPropertyAnimation(this, "windowOpacity", this);
    anim->setDuration(180); anim->setStartValue(1.0); anim->setEndValue(0.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &QPropertyAnimation::finished, this, [this]{ setWindowOpacity(1.0); showMinimized(); });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}
void CorevexWindow::refreshBookmarkButton() {
    // v4.1 STAR FIX: pehle star click par sirf tooltip badalta tha, icon me
    // koi farq nahi dikhta tha, isliye user ko laga button kaam hi nahi karta.
    // Ab bookmarked page par star ka background orange (shaded) hoga, aur
    // new-tab/search page par button disabled hoga (bookmark nahi ho sakta).
    if (!bookmarkBtn_) return;
    auto* view = currentView();
    if (!view || !store_ || !store_->isOpen() || !corevex::isSafeWebUrl(view->url())) {
        bookmarkBtn_->setEnabled(false);
        bookmarkBtn_->setStyleSheet("background:transparent; border:none; border-radius:15px;");
        bookmarkBtn_->setToolTip("Is page ko bookmark nahi kar sakte (new tab / search page)");
        return;
    }
    bookmarkBtn_->setEnabled(true);
    const QString url = view->url().toString();
    if (store_->isBookmarked(url)) {
        bookmarkBtn_->setStyleSheet("background:#FF5A1F; border:none; border-radius:15px;");
        bookmarkBtn_->setToolTip("Bookmarked ★ — dobara dabao to remove ho jayega");
    } else {
        bookmarkBtn_->setStyleSheet("background:transparent; border:none; border-radius:15px;");
        bookmarkBtn_->setToolTip("Bookmark this page");
    }
}
void CorevexWindow::addBookmark() {
    auto* view = currentView();
    if (!view || !store_ || !store_->isOpen()) return;
    const QString url = view->url().toString();
    if (!corevex::isSafeWebUrl(view->url())) { refreshBookmarkButton(); return; }
    if (store_->isBookmarked(url)) store_->removeBookmark(url);
    else store_->addBookmark(view->title(), url);
    refreshBookmarkButton();
    if (bookmarksPage_) { auto* pg = qobject_cast<corevex::BookmarksPage*>(bookmarksPage_); if (pg) pg->refresh(); }
}

void CorevexWindow::loadSettings() {
    // Purane keys bhi padhe jaate hain (backward compatible), par asli
    // config AiClient ko jaati hai — provider "local" / "api" / "gemini".
    aiProvider = settings->value("ai/provider", "local").toString();
    apiKey = settings->value("ai/apiKey", "").toString();
    baseUrl = settings->value("ai/baseUrl", "http://127.0.0.1:11434/api/chat").toString();
    // v3.3 migration: purana /api/generate endpoint glitched chat deta tha,
    // isliye saved /api/generate ko /api/chat me badal do (ek baar).
    if (baseUrl.endsWith(QStringLiteral("/api/generate"))) {
        baseUrl = baseUrl.left(baseUrl.size() - QStringLiteral("/api/generate").size()) + QStringLiteral("/api/chat");
        settings->setValue("ai/baseUrl", baseUrl);
    }
    aiModel = settings->value("ai/model", "llama3").toString();
    if (aiClient_) {
        corevex::AiConfig cfg;
        cfg.provider = aiProvider;
        cfg.baseUrl = baseUrl;
        cfg.model = aiModel;
        cfg.apiKey = apiKey;
        aiClient_->setConfig(cfg);
    }
    if (aiProviderCombo_ && aiProviderCombo_->currentText() != aiProvider) {
        aiProviderCombo_->blockSignals(true); aiProviderCombo_->setCurrentText(aiProvider); aiProviderCombo_->blockSignals(false);
        refreshAiModelCombo();
    }
    if (aiModelCombo_ && aiModelCombo_->currentText() != aiModel) {
        aiModelCombo_->blockSignals(true); aiModelCombo_->setCurrentText(aiModel); aiModelCombo_->blockSignals(false);
    }
}
void CorevexWindow::saveSettings() {
    settings->setValue("ai/provider", aiProvider);
    settings->setValue("ai/apiKey", apiKey);
    settings->setValue("ai/baseUrl", baseUrl);
    settings->setValue("ai/model", aiModel);
}

QWidget* createDarkStub(QString title) {
    QWidget *w = new QWidget();
    w->setStyleSheet("background: #0B0C0F;");
    QVBoxLayout *l = new QVBoxLayout(w);
    QLabel *lbl = new QLabel(title + " (System Framework)");
    lbl->setStyleSheet("color: #F4F4F5; font-size: 26px; font-weight: 600; font-family: 'Segoe UI';");
    lbl->setAlignment(Qt::AlignCenter);
    
    QLabel *sub = new QLabel("UI component registered successfully. Awaiting data binding.");
    sub->setStyleSheet("color: #A1A1AA; font-size: 14px; font-family: 'Segoe UI';");
    sub->setAlignment(Qt::AlignCenter);

    l->addStretch();
    l->addWidget(lbl);
    l->addWidget(sub);
    l->addStretch();
    return w;
}

// --- Section 8/9: asli pages (stub nahi) ---------------------------------
QWidget* CorevexWindow::createSearchPage() {
    return new corevex::SearchPage([this](const QString& q) {
        runCorevexSearch(q);
    }, this);
}
QWidget* CorevexWindow::createAiAssistantPage() {
    // Design System ki AI Assistant page: asli chat window, sirf button nahi.
    // Ye page bhi wahi AiClient + history use karta hai jo right panel use karta hai.
    auto* w = new QWidget();
    auto* lay = new QVBoxLayout(w);
    lay->setContentsMargins(24, 24, 24, 24); lay->setSpacing(12);
    { auto* pillRow = new QHBoxLayout(); auto* pill = new QLabel("AI ASSISTANT"); pill->setStyleSheet("background:#FF5A1F;color:#FFFFFF;border-radius:10px;padding:4px 12px;font-size:11px;font-weight:700;"); pill->setFixedHeight(20); pillRow->addWidget(pill); pillRow->addStretch(); lay->addLayout(pillRow); }
    auto* title = new QLabel("COREVEX AI"); title->setStyleSheet("color:#F4F4F5;font-size:26px;font-weight:700;");
    auto* sub = new QLabel("Local Ollama ya apni API se chat karo. Right panel wala AI bhi wahi history use karta hai.");
    sub->setWordWrap(true); sub->setStyleSheet("color:#A1A1AA;font-size:13px;");
    lay->addWidget(title); lay->addWidget(sub);
    auto* scroll = new QScrollArea(); scroll->setWidgetResizable(true);
    scroll->setStyleSheet("background:transparent; border:none;");
    auto* inner = new QWidget();
    auto* chatLay = new QVBoxLayout(inner); chatLay->addStretch();
    scroll->setWidget(inner);
    lay->addWidget(scroll, 1);
    auto* inputRow = new QHBoxLayout();
    auto* input = new QLineEdit(); input->setPlaceholderText("Ask anything...");
    input->setFixedHeight(44);
    input->setStyleSheet("background:#14161B; color:#F4F4F5; border:1px solid #2A2D35; border-radius:22px; padding:6px 18px;");
    auto* sendBtn = new QPushButton("Send"); sendBtn->setFixedSize(100, 44);
    sendBtn->setStyleSheet("background:#FF5A1F;color:#FFFFFF;border:none;border-radius:22px;font-weight:700;");
    inputRow->addWidget(input, 1); inputRow->addWidget(sendBtn);
    lay->addLayout(inputRow);
    auto send = [this, chatLay, input]() {
        QString txt = input->text().trimmed();
        if (txt.isEmpty() || !aiClient_) return;
        auto* uMsg = new QLabel(txt); uMsg->setWordWrap(true);
        uMsg->setStyleSheet("background:#FF5A1F; color:white; border-radius:10px; padding:10px; margin-left:60px;");
        chatLay->insertWidget(chatLay->count()-1, uMsg);
        input->clear();
        auto* aMsg = new QLabel("Thinking…"); aMsg->setWordWrap(true); aMsg->setTextInteractionFlags(Qt::TextSelectableByMouse);
        aMsg->setStyleSheet("background:#1B1E24; color:#F4F4F5; border:1px solid #2A2D35; border-radius:10px; padding:10px; margin-right:60px;");
        chatLay->insertWidget(chatLay->count()-1, aMsg);
        loadSettings();
        aiClient_->chat(aiHistory_, txt,
            [this, aMsg, txt](const QString& reply) {
                aMsg->setTextFormat(Qt::RichText);
                aMsg->setText(corevex::aiMarkdownToHtml(reply));
                saveAiChatHistory();
                aiHistory_.append({QStringLiteral("user"), txt});
                aiHistory_.append({QStringLiteral("assistant"), reply});
                if (aiHistory_.size() > 40) aiHistory_.removeFirst();
            },
            [aMsg](const QString& err) { aMsg->setText(QStringLiteral("AI Error: ") + err); });
    };
    connect(sendBtn, &QPushButton::clicked, this, send);
    connect(input, &QLineEdit::returnPressed, this, send);
    return w;
}
QWidget* CorevexWindow::createLibraryPage() {
    // Research page — Design System jaisa grid: Bookmarks, History, Downloads, ADS Lab.
    auto* w = new QWidget();
    auto* outer = new QHBoxLayout(w); outer->setContentsMargins(0,0,0,0); outer->addStretch();
    auto* central = new QWidget(); central->setMaximumWidth(800); central->setMinimumWidth(360);
    auto* lay = new QVBoxLayout(central); lay->setContentsMargins(24,28,24,24); lay->setSpacing(14);
    outer->addWidget(central, 1); outer->addStretch();
    { auto* pillRow = new QHBoxLayout(); auto* pill = new QLabel("RESEARCH"); pill->setStyleSheet("background:#FF5A1F;color:#FFFFFF;border-radius:10px;padding:4px 12px;font-size:11px;font-weight:700;"); pill->setFixedHeight(20); pillRow->addWidget(pill); pillRow->addStretch(); lay->addLayout(pillRow); }
    auto* title = new QLabel("Research"); title->setStyleSheet("color:#F4F4F5;font-size:26px;font-weight:700;");
    auto* lbl = new QLabel("Tumhara research ek jagah — saved sites, history, downloads aur ADS Lab.");
    lbl->setWordWrap(true); lbl->setStyleSheet("color:#A1A1AA;font-size:14px;");
    lay->addWidget(title); lay->addWidget(lbl);
    auto* grid = new QGridLayout(); grid->setSpacing(14);
    auto mkCard = [&](const QString& t, const QString& s, const QString& count, int idx) -> QFrame* {
        auto* card = new QFrame(); card->setStyleSheet("QFrame{background:#14161B;border:1px solid #2A2D35;border-radius:12px;}");
        auto* cl = new QVBoxLayout(card); cl->setContentsMargins(18,16,18,16); cl->setSpacing(8);
        auto* ct = new QLabel(t); ct->setStyleSheet("color:#F4F4F5;font-size:16px;font-weight:700;");
        auto* cc = new QLabel(count); cc->setStyleSheet("color:#FF5A1F;font-size:20px;font-weight:700;");
        auto* cs = new QLabel(s); cs->setWordWrap(true); cs->setStyleSheet("color:#A1A1AA;font-size:13px;");
        auto* cb = new QPushButton("Kholo  →"); cb->setFixedSize(120, 36);
        cb->setStyleSheet("background:#FF5A1F;color:#FFFFFF;border:none;border-radius:18px;font-weight:700;");
        connect(cb, &QPushButton::clicked, this, [this, idx]{ mainStack->setCurrentIndex(idx); });
        cl->addWidget(ct); cl->addWidget(cc); cl->addWidget(cs); cl->addWidget(cb);
        return card;
    };
    const int bmCount = store_ && store_->isOpen() ? store_->bookmarks().size() : 0;
    const int histCount = store_ && store_->isOpen() ? store_->history(100).size() : 0;
    const int dlCount = QDir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).entryList(QDir::Files).size();
    grid->addWidget(mkCard("Bookmarks", "Star karke save ki hui sites.", QString::number(bmCount) + " saved", 4), 0, 0);
    grid->addWidget(mkCard("History", "Ab tak kholi hui sites.", QString::number(histCount) + " pages", 5), 0, 1);
    grid->addWidget(mkCard("Downloads", "Downloads folder ki files.", QString::number(dlCount) + " files", 6), 1, 0);
    grid->addWidget(mkCard("ADS Lab", "Trie, Heap, Hash, Graph, AVL, Index.", "6 structures", 7), 1, 1);
    lay->addLayout(grid);
    lay->addStretch();
    return w;
}
QWidget* CorevexWindow::createBookmarksPage() {
    return new corevex::BookmarksPage(store_, [this](const QString& url) {
        mainStack->setCurrentIndex(0);
        addNewTab(QUrl(url));
    }, this);
}
QWidget* CorevexWindow::createHistoryPage() {
    return new corevex::HistoryPage(store_, [this](const QString& url) {
        mainStack->setCurrentIndex(0);
        addNewTab(QUrl(url));
    }, this);
}
QWidget* CorevexWindow::createDownloadsPage() { return new corevex::DownloadsPage(this); }
QWidget* CorevexWindow::createADSLabPage() { return new corevex::AdsLabPage(this); }
QWidget* CorevexWindow::createSettingsPage() {
    return new corevex::SettingsPage([this]{ openAiSettings(); }, this);
}
QWidget* CorevexWindow::createMadeByPage() {
    // Team page — user ne diye hue naam/IDs, centered cards, no empty bottom.
    auto* w = new QWidget();
    auto* outer = new QHBoxLayout(w); outer->setContentsMargins(0,0,0,0); outer->addStretch();
    auto* central = new QWidget(); central->setMaximumWidth(720); central->setMinimumWidth(360);
    auto* lay = new QVBoxLayout(central); lay->setContentsMargins(24,24,24,24); lay->setSpacing(14);
    outer->addWidget(central, 1); outer->addStretch();
    { auto* pillRow = new QHBoxLayout(); pillRow->addStretch(); auto* pill = new QLabel("TEAM CREDITS"); pill->setStyleSheet("background:#FF5A1F;color:#FFFFFF;border-radius:10px;padding:4px 12px;font-size:11px;font-weight:700;"); pill->setFixedHeight(20); pillRow->addWidget(pill); pillRow->addStretch(); lay->addLayout(pillRow); }
    auto* logo = new QLabel(); logo->setPixmap(QIcon(corevex::assetPath("logos/corevex_wordmark.svg")).pixmap(200, 44));
    logo->setAlignment(Qt::AlignHCenter);
    auto* title = new QLabel("Search. Browse. Think."); title->setAlignment(Qt::AlignHCenter);
    title->setStyleSheet("color:#FF5A1F;font-size:18px;font-weight:700;");
    auto* body = new QLabel("C++ / Qt Widgets / Qt WebEngine par bana hua AI browser. Theme, search, AI, bookmarks/history aur ADS Lab — sab modules me divided hai, har module ke upar SECTION comment me uska kaam likha hai.");
    body->setWordWrap(true); body->setAlignment(Qt::AlignHCenter); body->setStyleSheet("color:#A1A1AA;font-size:14px;");
    auto* teamTitle = new QLabel("Made By"); teamTitle->setStyleSheet("color:#F4F4F5;font-size:20px;font-weight:700;");
    // Roles hata diye hain — user ne kaha sabke roles hata do, kisi par Lead Developer nahi.
    auto mkMember = [&](const QString& name, const QString& id) -> QFrame* {
        auto* card = new QFrame(); card->setStyleSheet("QFrame{background:#14161B;border:1px solid #2A2D35;border-radius:12px;}");
        auto* cl = new QHBoxLayout(card); cl->setContentsMargins(18,14,18,14);
        auto* avatar = new QLabel(name.left(1)); avatar->setFixedSize(44, 44); avatar->setAlignment(Qt::AlignCenter);
        avatar->setStyleSheet("background:#FF5A1F;color:#FFFFFF;border-radius:22px;font-size:18px;font-weight:700;");
        auto* txt = new QVBoxLayout(); txt->setSpacing(2);
        auto* n = new QLabel(name); n->setStyleSheet("color:#F4F4F5;font-size:16px;font-weight:700;");
        auto* i = new QLabel(id); i->setStyleSheet("color:#A1A1AA;font-size:13px;");
        txt->addWidget(n); txt->addWidget(i);
        cl->addWidget(avatar); cl->addLayout(txt, 1);
        return card;
    };
    lay->addStretch();
    lay->addWidget(logo); lay->addWidget(title); lay->addWidget(body);
    lay->addWidget(teamTitle);
    lay->addWidget(mkMember("Aarit", "25BCS10003"));
    lay->addWidget(mkMember("Dhruv", "25BCS10405"));
    lay->addWidget(mkMember("Maithli", "25BCS10376"));
    lay->addStretch();
    return w;
}