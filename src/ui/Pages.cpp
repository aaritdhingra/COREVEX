// ============================================================================
// SECTION 8 (impl) — REAL SIDEBAR PAGES, CHROME-STYLE
// ----------------------------------------------------------------------------
// Layout rule (har page par same):
//   Outer HBox: stretch | central (max 800px) | stretch
//   Central VBox: title, subtitle, search, list/cards, footer.
//   * List pages me list expanding hai, isliye neeche khaali gap nahi.
//   * Short pages me central content ke upar/neeche stretch hai,
//     isliye content screen ke beech me aata hai, top par chipka nahi.
// ============================================================================
#include "Pages.h"

#include <QComboBox>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QWebEngineProfile>
#include <QWebEngineCookieStore>
#include <QCheckBox>
#include <QMessageBox>
#include <QFileInfo>
#include <QSettings>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStandardPaths>
#include <QUrl>
#include <QVBoxLayout>

#include "../search/SearchEngine.h"
#include "../storage/Store.h"

namespace corevex {

namespace {

// Central container banata hai: max 800px, poori height.
// Returns central widget, jiska layout caller bharega.
QWidget* centered(QWidget* page, QVBoxLayout** outLay) {
    auto* outer = new QHBoxLayout(page);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);
    outer->addStretch();
    auto* central = new QWidget(page);
    central->setMaximumWidth(800);
    central->setMinimumWidth(360);
    central->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    auto* lay = new QVBoxLayout(central);
    lay->setContentsMargins(28, 32, 28, 28);
    lay->setSpacing(16);
    outer->addWidget(central, 1);
    outer->addStretch();
    *outLay = lay;
    return central;
}

QLabel* orangePill(const QString& text) {
    auto* l = new QLabel(text.toUpper());
    l->setStyleSheet(QStringLiteral("background:#FF5A1F;color:#FFFFFF;border-radius:10px;padding:4px 12px;font-size:11px;font-weight:700;"));
    l->setFixedHeight(20);
    return l;
}
QLabel* titleLabel(const QString& text) {
    auto* l = new QLabel(text);
    l->setStyleSheet(QStringLiteral("color:#F4F4F5;font-size:28px;font-weight:800;letter-spacing:-0.5px;"));
    return l;
}
QLabel* subtitleLabel(const QString& text) {
    auto* l = new QLabel(text);
    l->setWordWrap(true);
    l->setStyleSheet(QStringLiteral("color:#A1A1AA;font-size:13px;"));
    return l;
}
QLineEdit* searchInput(const QString& placeholder) {
    auto* e = new QLineEdit();
    e->setPlaceholderText(placeholder);
    e->setFixedHeight(44);
    e->setStyleSheet(QStringLiteral("background:#14161B;color:#F4F4F5;border:1px solid #2A2D35;border-radius:22px;padding:6px 18px;font-size:14px;"));
    return e;
}
QPushButton* primaryButton(const QString& text, QWidget* parent, int width = 180) {
    auto* b = new QPushButton(text, parent);
    b->setFixedSize(width, 40);
    b->setStyleSheet(QStringLiteral("background:#FF5A1F;color:#FFFFFF;border:none;border-radius:20px;font-weight:700;"));
    return b;
}
QPushButton* secondaryButton(const QString& text, QWidget* parent, int width = 160) {
    auto* b = new QPushButton(text, parent);
    b->setFixedSize(width, 36);
    b->setStyleSheet(QStringLiteral("background:#1B1E24;color:#F4F4F5;border:1px solid #2A2D35;border-radius:18px;"));
    return b;
}
QListWidget* cardList() {
    auto* list = new QListWidget();
    list->setStyleSheet(QStringLiteral(
        "QListWidget{background:transparent;border:none;}"
        "QListWidget::item{background:#14161B;border:1px solid #2A2D35;border-radius:12px;padding:12px 16px;margin-bottom:10px;color:#F4F4F5;}"
        "QListWidget::item:selected{background:#1B1E24;border-color:#FF5A1F;color:#FFFFFF;}"
        "QListWidget::item:hover{border-color:#FF5A1F;}"));
    list->setSpacing(0);
    return list;
}
QString formatSize(qint64 bytes) {
    if (bytes < 1024) return QString::number(bytes) + QStringLiteral(" B");
    if (bytes < 1024 * 1024) return QString::number(bytes / 1024.0, 'f', 0) + QStringLiteral(" KB");
    if (bytes < 1024 * 1024 * 1024) return QString::number(bytes / 1048576.0, 'f', 1) + QStringLiteral(" MB");
    return QString::number(bytes / 1073741824.0, 'f', 2) + QStringLiteral(" GB");
}
QFrame* infoCard(const QString& title, const QString& body) {
    auto* f = new QFrame();
    f->setStyleSheet(QStringLiteral("QFrame{background:#14161B;border:1px solid #2A2D35;border-radius:14px;}"));
    auto* lay = new QVBoxLayout(f);
    lay->setContentsMargins(18, 16, 18, 16);
    lay->setSpacing(6);
    auto* t = new QLabel(title);
    t->setStyleSheet(QStringLiteral("color:#F4F4F5;font-size:15px;font-weight:700;"));
    t->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    auto* b = new QLabel(body);
    b->setWordWrap(true);
    b->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    b->setStyleSheet(QStringLiteral("color:#A1A1AA;font-size:13px;"));
    lay->addWidget(t); lay->addWidget(b);
    return f;
}

}  // namespace

// --- Bookmarks (GRID, Design System jaisa) -------------------------------
BookmarksPage::BookmarksPage(Store* store, std::function<void(const QString&)> openUrl, QWidget* parent)
    : QWidget(parent), store_(store), openUrl_(std::move(openUrl)) {
    QVBoxLayout* lay = nullptr;
    centered(this, &lay);
    { auto* pillRow = new QHBoxLayout(); pillRow->addWidget(orangePill(QStringLiteral("Bookmarks"))); pillRow->addStretch();
      auto* exportBtn = secondaryButton(QStringLiteral("Export HTML"), this, 130);
      exportBtn->setToolTip("Bookmarks ko HTML file me export karo (dusre browser me import kar sakte ho)");
      connect(exportBtn, &QPushButton::clicked, this, [this] {
          if (!store_ || !store_->isOpen()) return;
          const QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Export Bookmarks"), QStringLiteral("bookmarks.html"), QStringLiteral("HTML Files (*.html)"));
          if (path.isEmpty()) return;
          QFile file(path);
          if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
          QTextStream out(&file);
          out << "<!DOCTYPE NETSCAPE-Bookmark-file-1>\n<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=UTF-8\">\n<TITLE>Bookmarks</TITLE>\n<H1>Bookmarks</H1>\n<DL><p>\n";
          for (const auto& b : store_->bookmarks())
              out << "<DT><A HREF=\"" << b.url << "\">" << (b.title.isEmpty() ? b.url : b.title) << "</A>\n";
          out << "</DL><p>\n";
          file.close();
          QMessageBox::information(this, QStringLiteral("Exported"), QStringLiteral("Bookmarks export ho gaye."));
      });
      pillRow->addWidget(exportBtn); lay->addLayout(pillRow); }
    lay->addWidget(titleLabel(QStringLiteral("Bookmarks")));
    lay->addWidget(subtitleLabel(QStringLiteral("Saved sites — grid cards. Kisi card par Open dabao, site browser me khul jayegi.")));
    filter_ = searchInput(QStringLiteral("Search bookmarks…"));
    lay->addWidget(filter_);
    connect(filter_, &QLineEdit::textChanged, this, &BookmarksPage::applyFilter);
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet(QStringLiteral("QScrollArea{background:transparent;border:none;}"));
    gridContainer_ = new QWidget();
    grid_ = new QGridLayout(gridContainer_);
    grid_->setContentsMargins(0, 0, 0, 0);
    grid_->setSpacing(14);
    grid_->setAlignment(Qt::AlignTop);
    grid_->setColumnStretch(0, 1); grid_->setColumnStretch(1, 1); grid_->setColumnStretch(2, 1);
    scroll->setWidget(gridContainer_);
    lay->addWidget(scroll, 1);
    refresh();
}
void BookmarksPage::applyFilter() {
    const QString q = filter_ ? filter_->text().trimmed().toLower() : QString();
    for (int i = 0; i < grid_->count(); ++i) {
        auto* item = grid_->itemAt(i);
        if (item && item->widget()) {
            const QString txt = item->widget()->property("searchText").toString().toLower();
            item->widget()->setHidden(!q.isEmpty() && !txt.contains(q));
        }
    }
}
void BookmarksPage::refresh() {
    // Purani cards hatao.
    while (grid_->count() > 0) {
        auto* item = grid_->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    if (!store_ || !store_->isOpen()) {
        grid_->addWidget(new QLabel(QStringLiteral("Store unavailable — bookmarks save nahi ho rahe.")), 0, 0);
        return;
    }
    const auto list = store_->bookmarks();
    if (list.isEmpty()) {
        grid_->addWidget(new QLabel(QStringLiteral("Abhi koi bookmark nahi. Kisi page par star dabao.")), 0, 0);
        return;
    }
    static const char* kColors[] = {"#FF5A1F", "#2563EB", "#7C3AED", "#059669", "#DB2777", "#D97706"};
    int row = 0, col = 0;
    for (const auto& b : list) {
        auto* card = new QFrame();
        card->setFixedHeight(230);
        card->setStyleSheet(QStringLiteral("QFrame{background:#14161B;border:1px solid #2A2D35;border-radius:14px;}"));
        card->setProperty("searchText", b.title + " " + b.url);
        auto* cl = new QVBoxLayout(card);
        cl->setContentsMargins(0, 0, 0, 12); cl->setSpacing(8);
        // Thumbnail tile: domain ka pehla letter, colored background.
        QString host = QUrl(b.url).host();
        if (host.isEmpty()) host = b.url;
        if (host.startsWith(QStringLiteral("www."))) host = host.mid(4);
        auto* tile = new QLabel(host.left(1).toUpper());
        tile->setFixedHeight(90);
        tile->setAlignment(Qt::AlignCenter);
        tile->setStyleSheet(QString("background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 %1,stop:1 #F02D24);color:#FFFFFF;border-top-left-radius:12px;border-top-right-radius:12px;font-size:32px;font-weight:700;").arg(kColors[qHash(b.url) % 6]));
        auto* title = new QLabel(b.title.isEmpty() ? host : b.title);
        title->setWordWrap(true);
        title->setStyleSheet(QStringLiteral("color:#F4F4F5;font-size:14px;font-weight:700;padding:0 12px;"));
        auto* urlLbl = new QLabel(host);
        urlLbl->setWordWrap(true);
        urlLbl->setStyleSheet(QStringLiteral("color:#A1A1AA;font-size:12px;padding:0 12px;"));
        auto* openBtn = new QPushButton(QStringLiteral("Open  →"));
        openBtn->setFixedHeight(32);
        openBtn->setStyleSheet(QStringLiteral("background:#1B1E24;color:#F4F4F5;border:1px solid #2A2D35;border-radius:16px;margin:0 12px;"));
        connect(openBtn, &QPushButton::clicked, this, [this, b] { if (openUrl_) openUrl_(b.url); });
        cl->addWidget(tile); cl->addWidget(title); cl->addWidget(urlLbl); cl->addWidget(openBtn);
        grid_->addWidget(card, row, col);
        if (++col >= 3) { col = 0; ++row; }
    }
    applyFilter();
}

// --- History ---------------------------------------------------------------
HistoryPage::HistoryPage(Store* store, std::function<void(const QString&)> openUrl, QWidget* parent)
    : QWidget(parent), store_(store), openUrl_(std::move(openUrl)) {
    QVBoxLayout* lay = nullptr;
    centered(this, &lay);
    { auto* pillRow = new QHBoxLayout(); pillRow->addWidget(orangePill(QStringLiteral("History"))); pillRow->addStretch(); lay->addLayout(pillRow); }
    lay->addWidget(titleLabel(QStringLiteral("History")));
    lay->addWidget(subtitleLabel(QStringLiteral("Jo pages khole, wo yahan newest-first dikhenge.")));
    filter_ = searchInput(QStringLiteral("Search history…"));
    lay->addWidget(filter_);
    connect(filter_, &QLineEdit::textChanged, this, &HistoryPage::applyFilter);
    list_ = cardList();
    lay->addWidget(list_, 1);
    auto* row = new QHBoxLayout();
    auto* clear = secondaryButton(QStringLiteral("Clear History"), this);
    row->addWidget(clear);
    row->addStretch();
    lay->addLayout(row);
    connect(clear, &QPushButton::clicked, this, [this] { if (store_) store_->clearHistory(); refresh(); });
    // v4.1: History me EK click par hi naye tab me khule (Chrome jaisa single-click,
    // pehle double-click chahiye tha, isliye user ko lagta tha click kaam nahi karta).
    connect(list_, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        if (openUrl_) openUrl_(item->data(Qt::UserRole).toString());
    });
    refresh();
}
void HistoryPage::applyFilter() {
    const QString q = filter_ ? filter_->text().trimmed().toLower() : QString();
    for (int i = 0; i < list_->count(); ++i) {
        auto* item = list_->item(i);
        item->setHidden(!q.isEmpty() && !item->text().toLower().contains(q));
    }
}
void HistoryPage::refresh() {
    list_->clear();
    if (!store_ || !store_->isOpen()) { list_->addItem(QStringLiteral("Store unavailable.")); return; }
    const auto list = store_->history(100);
    if (list.isEmpty()) list_->addItem(QStringLiteral("Abhi koi history nahi."));
    for (const auto& h : list) {
        auto* item = new QListWidgetItem(h.title + QStringLiteral("\n") + h.time.toString(QStringLiteral("dd MMM, hh:mm")) + QStringLiteral("  •  ") + h.url, list_);
        item->setData(Qt::UserRole, h.url);
        item->setToolTip(h.url);
    }
    applyFilter();
}

// --- Downloads (Chrome-style) ----------------------------------------------
DownloadsPage::DownloadsPage(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* lay = nullptr;
    centered(this, &lay);
    { auto* pillRow = new QHBoxLayout(); pillRow->addWidget(orangePill(QStringLiteral("Downloads"))); pillRow->addStretch(); lay->addLayout(pillRow); }
    lay->addWidget(titleLabel(QStringLiteral("Downloads")));
    lay->addWidget(subtitleLabel(QStringLiteral("Browser se download ki hui files (Downloads folder). Ek click par file khulegi.")));
    filter_ = searchInput(QStringLiteral("Search downloads…"));
    lay->addWidget(filter_);
    connect(filter_, &QLineEdit::textChanged, this, &DownloadsPage::applyFilter);
    list_ = cardList();
    lay->addWidget(list_, 1);
    auto* row = new QHBoxLayout();
    auto* refreshBtn = secondaryButton(QStringLiteral("Refresh"), this, 120);
    auto* folderBtn = secondaryButton(QStringLiteral("Open Folder"), this, 140);
    row->addWidget(refreshBtn);
    row->addWidget(folderBtn);
    row->addStretch();
    lay->addLayout(row);
    connect(refreshBtn, &QPushButton::clicked, this, &DownloadsPage::refresh);
    connect(folderBtn, &QPushButton::clicked, this, [] {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)));
    });
    // v4.1: Downloads me EK click par hi file khule (pehle double-click tha).
    connect(list_, &QListWidget::itemClicked, this, [](QListWidgetItem* item) {
        const QString path = item->data(Qt::UserRole).toString();
        if (!path.isEmpty()) QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    });
    refresh();
}
void DownloadsPage::applyFilter() {
    const QString q = filter_ ? filter_->text().trimmed().toLower() : QString();
    for (int i = 0; i < list_->count(); ++i) {
        auto* item = list_->item(i);
        item->setHidden(!q.isEmpty() && !item->text().toLower().contains(q));
    }
}
void DownloadsPage::refresh() {
    list_->clear();
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    const auto files = QDir(dir).entryInfoList(QDir::Files, QDir::Time);
    if (files.isEmpty()) { list_->addItem(QStringLiteral("Downloads folder khaali hai.")); return; }
    for (const auto& f : files) {
        const QString ext = f.suffix().toUpper();
        auto* item = new QListWidgetItem(
            f.fileName() + QStringLiteral("\n") +
            (ext.isEmpty() ? QStringLiteral("File") : ext) + QStringLiteral("  •  ") +
            formatSize(f.size()) + QStringLiteral("  •  ") +
            f.lastModified().toString(QStringLiteral("dd MMM yyyy, hh:mm")), list_);
        item->setData(Qt::UserRole, f.absoluteFilePath());
        item->setToolTip(f.absoluteFilePath());
    }
    applyFilter();
}

// --- Search (centered, Chrome new-tab jaisa) --------------------------------
SearchPage::SearchPage(std::function<void(const QString&)> onSearch, QWidget* parent)
    : QWidget(parent), onSearch_(std::move(onSearch)) {
    QVBoxLayout* lay = nullptr;
    centered(this, &lay);
    // v3.3: Top/bottom stretch HATA diya — wahi stretch page ko beech me
    // latka deta tha aur neeche itni negative space dikhti thi. Ab page
    // upar se shuru hota hai aur neeche Try/How sections se bhara rehta hai.
    { auto* pillRow = new QHBoxLayout(); pillRow->addStretch(); pillRow->addWidget(orangePill(QStringLiteral("Search"))); pillRow->addStretch(); lay->addLayout(pillRow); }
    lay->addWidget(titleLabel(QStringLiteral("Search")), 0, Qt::AlignHCenter);
    // Subtitle ko AlignHCenter MAT do — wordWrap wale QLabel ka height
    // centered alignment me galat calculate hota hai aur text kat jaata hai
    // (screenshot me subtitle ki aakhri line search box ke peeche kat rahi thi).
    lay->addWidget(subtitleLabel(QStringLiteral("Query likho, Enter dabao — COREVEX apne theme me results dikhayega.")));
    auto* row = new QHBoxLayout();
    auto* input = new QLineEdit(this);
    input->setPlaceholderText(QStringLiteral("Search the web…"));
    input->setFixedHeight(48);
    input->setStyleSheet(QStringLiteral("background:#14161B;color:#F4F4F5;border:1px solid #2A2D35;border-radius:24px;padding:6px 20px;font-size:14px;"));
    auto* btn = primaryButton(QStringLiteral("Search"), this, 130);
    btn->setFixedHeight(48);
    btn->setStyleSheet(QStringLiteral("background:#FF5A1F;color:#FFFFFF;border:none;border-radius:24px;font-weight:700;"));
    row->addWidget(input, 1);
    row->addWidget(btn);
    lay->addLayout(row);
    auto run = [this, input] { if (onSearch_ && !input->text().trimmed().isEmpty()) onSearch_(input->text().trimmed()); };
    connect(btn, &QPushButton::clicked, this, run);
    connect(input, &QLineEdit::returnPressed, this, run);
    auto* chips = new QHBoxLayout();
    chips->addStretch();
    for (const char* chipText : {"Research", "Summarize Page", "Compare Sources", "Find Information"}) {
        auto* chip = new QPushButton(QString::fromLatin1(chipText), this);
        chip->setFixedHeight(34);
        chip->setStyleSheet(QStringLiteral("background:#1B1E24;color:#A1A1AA;border:1px solid #2A2D35;border-radius:17px;padding:6px 16px;"));
        connect(chip, &QPushButton::clicked, this, [this, chipText] { if (onSearch_) onSearch_(QString::fromLatin1(chipText)); });
        chips->addWidget(chip);
    }
    chips->addStretch();
    lay->addLayout(chips);
    // Try these — page khaali na lage, negative space bharne ke liye.
    lay->addWidget(titleLabel(QStringLiteral("Try these")), 0, Qt::AlignLeft);
    auto* tryRow = new QHBoxLayout();
    for (const char* q : {"Weather today", "Latest news", "Cricket score", "Learn C++"}) {
        auto* b = new QPushButton(QString::fromLatin1(q), this);
        b->setFixedHeight(36);
        b->setStyleSheet(QStringLiteral("background:#14161B;color:#F4F4F5;border:1px solid #2A2D35;border-radius:18px;padding:6px 14px;"));
        connect(b, &QPushButton::clicked, this, [this, q] { if (onSearch_) onSearch_(QString::fromLatin1(q)); });
        tryRow->addWidget(b);
    }
    tryRow->addStretch();
    lay->addLayout(tryRow);
    lay->addStretch();
}

// --- Settings (centered cards, no empty bottom) -----------------------------
SettingsPage::SettingsPage(std::function<void()> openAiSettings, QWidget* parent) : QWidget(parent) {
    // v4.1 SETTINGS TEXT-CUT FIX: Settings page lambi hai (5 cards), window chhota
    // hone par layout cards ko squeeze kar deta tha — title/body text kat jaata tha
    // aur line ke through dikhta tha. Ab poora page QScrollArea me hai, cards apni
    // natural height me rahenge, neeche scroll karke poora text dikhega.
    QVBoxLayout* lay = nullptr;
    auto* outerLay = new QVBoxLayout(this);
    outerLay->setContentsMargins(0, 0, 0, 0);
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet(QStringLiteral("QScrollArea{background:transparent;border:none;}"));
    auto* inner = new QWidget();
    centered(inner, &lay);
    scroll->setWidget(inner);
    outerLay->addWidget(scroll);
    { auto* pillRow = new QHBoxLayout(); pillRow->addWidget(orangePill(QStringLiteral("Settings"))); pillRow->addStretch(); lay->addLayout(pillRow); }
    lay->addWidget(titleLabel(QStringLiteral("Settings")));
    lay->addWidget(subtitleLabel(QStringLiteral("AI provider, local LLM ya apni API key — sab yahan se.")));
    auto* aiCard = infoCard(QStringLiteral("AI Assistant"),
        QStringLiteral("Local Ollama (default) ya apni API key se chat. Settings dialog me provider, base URL, model aur key badlo."));
    auto* aiBtn = primaryButton(QStringLiteral("AI Settings kholo…"), this, 220);
    qobject_cast<QVBoxLayout*>(aiCard->layout())->addWidget(aiBtn);
    connect(aiBtn, &QPushButton::clicked, this, openAiSettings);
    lay->addWidget(aiCard);
    auto* engCard = infoCard(QStringLiteral("Search Engine"),
        QStringLiteral("COREVEX apna search page dikhata hai. Source neeche se chuno: Brave ke links direct hote hain, Bing aur Mojeek fallback hain. Google chunne par seedha Google ka page khulega."));
    auto* engCombo = new QComboBox(this);
    engCombo->addItem(QStringLiteral("Google — seedha Google page (sorry-block ho sakta hai)"), QStringLiteral("google"));
    engCombo->addItem(QStringLiteral("Brave — default, direct links"), QStringLiteral("brave"));
    engCombo->addItem(QStringLiteral("Mojeek — fallback"), QStringLiteral("mojeek"));
    engCombo->addItem(QStringLiteral("Bing"), QStringLiteral("bing"));
    engCombo->setCurrentIndex(engCombo->findData(searchEngineId()));
    engCombo->setFixedSize(360, 36);
    connect(engCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [engCombo](int) {
        setSearchEngineId(engCombo->currentData().toString());
    });
    qobject_cast<QVBoxLayout*>(engCard->layout())->addWidget(engCombo);
    lay->addWidget(engCard);
    // --- Background card (bg.png chooser) ------------------------------
    // User ne bg.png diya hai, wo ab assets me embedded hai, isliye
    // default me wahi lagega. Phir bhi chooser hai, taaki baad me koi
    // aur image chuni ja sake — choice QSettings me save hoti hai aur
    // naya tab kholne par lagti hai (purana tab reload karo).
    auto* bgCard = infoCard(QStringLiteral("Background"),
        QStringLiteral("Home page ka background. Default: tumhara diya hua bg.png (lava). Koi aur image chuno to wo naya tab me lagegi."));
    auto* bgLay = qobject_cast<QVBoxLayout*>(bgCard->layout());
    auto* bgLabel = new QLabel(this);
    {
        QSettings s(QStringLiteral("COREVEX"), QStringLiteral("Browser"));
        const QString chosen = s.value(QStringLiteral("background/imagePath")).toString();
        bgLabel->setText(chosen.isEmpty() ? QStringLiteral("Current: bg.png (default)") : QStringLiteral("Current: ") + chosen);
    }
    bgLabel->setWordWrap(true);
    bgLabel->setStyleSheet(QStringLiteral("color:#A1A1AA;font-size:13px;"));
    bgLay->addWidget(bgLabel);
    auto* bgRow = new QHBoxLayout();
    auto* bgBtn = secondaryButton(QStringLiteral("Image chuno…"), this, 160);
    auto* bgReset = secondaryButton(QStringLiteral("Default wapas"), this, 160);
    bgRow->addWidget(bgBtn); bgRow->addWidget(bgReset); bgRow->addStretch();
    bgLay->addLayout(bgRow);
    connect(bgBtn, &QPushButton::clicked, this, [bgLabel] {
        const QString path = QFileDialog::getOpenFileName(nullptr, QStringLiteral("Background image chuno"), QString(), QStringLiteral("Images (*.png *.jpg *.jpeg *.svg)"));
        if (path.isEmpty()) return;
        QSettings s(QStringLiteral("COREVEX"), QStringLiteral("Browser"));
        s.setValue(QStringLiteral("background/imagePath"), path);
        bgLabel->setText(QStringLiteral("Current: ") + path + QStringLiteral("  (naya tab kholo)"));
    });
    connect(bgReset, &QPushButton::clicked, this, [bgLabel] {
        QSettings s(QStringLiteral("COREVEX"), QStringLiteral("Browser"));
        s.remove(QStringLiteral("background/imagePath"));
        bgLabel->setText(QStringLiteral("Current: bg.png (default)"));
    });
    lay->addWidget(bgCard);
    // --- Production Settings (v3.3) --------------------------------------
    auto* prodCard = infoCard(QStringLiteral("Browser — Production"),
        QStringLiteral("Session restore, shortcuts aur browsing data — sab yahan se."));
    auto* prodLay = qobject_cast<QVBoxLayout*>(prodCard->layout());
    {
        QSettings s(QStringLiteral("COREVEX"), QStringLiteral("Browser"));
        auto* restoreChk = new QCheckBox(QStringLiteral("Startup par pichhle tabs wapas kholo (session restore)"), this);
        restoreChk->setChecked(s.value(QStringLiteral("session/restore"), true).toBool());
        restoreChk->setStyleSheet(QStringLiteral("color:#F4F4F5;"));
        connect(restoreChk, &QCheckBox::toggled, this, [](bool v) {
            QSettings ss(QStringLiteral("COREVEX"), QStringLiteral("Browser"));
            ss.setValue(QStringLiteral("session/restore"), v);
        });
        prodLay->addWidget(restoreChk);
    }
    auto* clearBtn = secondaryButton(QStringLiteral("Clear browsing data (history + cookies)"), this, 320);
    connect(clearBtn, &QPushButton::clicked, this, [this] {
        if (QMessageBox::question(this, QStringLiteral("Clear browsing data"), QStringLiteral("History, cookies aur visited links saaf kar dun? Bookmarks safe rahenge.")) != QMessageBox::Yes) return;
        if (auto* profile = QWebEngineProfile::defaultProfile()) {
            profile->clearAllVisitedLinks();
            profile->cookieStore()->deleteAllCookies();
        }
        // HistoryPage ka store yahan se direct nahi milta, isliye Settings
        // page sirf cookies/links saaf karta hai — history History page ke
        // Clear button se saaf hoti hai. Ye button phir bhi useful hai.
        QMessageBox::information(this, QStringLiteral("Done"), QStringLiteral("Cookies aur visited links saaf ho gaye. History ke liye History page par Clear dabao."));
    });
    prodLay->addWidget(clearBtn);
    auto* shortcutLbl = new QLabel(QStringLiteral("Shortcuts: Ctrl+T new tab  •  Ctrl+W close  •  Ctrl+L address  •  Ctrl+F find  •  Ctrl+Shift+T reopen tab  •  Ctrl+Shift+D duplicate  •  Ctrl + / - / 0 zoom"), this);
    shortcutLbl->setWordWrap(true);
    shortcutLbl->setStyleSheet(QStringLiteral("color:#A1A1AA;font-size:12px;"));
    prodLay->addWidget(shortcutLbl);
    lay->addWidget(prodCard);
    lay->addWidget(infoCard(QStringLiteral("About COREVEX"),
        QStringLiteral("COREVEX v4.5 — C++ / Qt Widgets / Qt WebEngine\nTheme: #0B0C0F background, #FF5A1F accent\nDefault AI: Local Ollama (http://127.0.0.1:11434)\nSearch: COREVEX apna search page — Brave/Bing fetch, last-image theme, AI Overview, Research tools, Related, Sources\nAI: Local Ollama / OpenAI API / Gemini — panel se provider+model picker, markdown, Summarize Page, Copy Last\nNew in v4.6: sleek toggle/tab X, tab X visible fix, Mojeek direct links, Bing parser/snippet fix, COREVEX all caps")));
    lay->addStretch();
}

}  // namespace corevex
