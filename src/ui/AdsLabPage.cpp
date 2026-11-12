// ============================================================================
// SECTION 9 (impl) — ADS LAB
// ============================================================================
#include "AdsLabPage.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QtMath>
#include <QPushButton>
#include <QVBoxLayout>
#include <algorithm>
#include <random>
#include <vector>

#include "../ads/balanced_tree/AvlTree.h"
#include "../ads/graph/EvidenceGraph.h"
#include "../ads/hash_table/HashTable.h"
#include "../ads/heap/BinaryHeap.h"
#include "../ads/inverted_index/InvertedIndex.h"
#include "../ads/trie/Trie.h"

namespace corevex {

// --- Bar chart (QPainter, no Qt Charts) ------------------------------------
BarChartWidget::BarChartWidget(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(180);
}
void BarChartWidget::setData(const QVector<QPair<QString, double>>& data) {
    data_ = data;
    update();
}
void BarChartWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor("#14161B"));
    if (data_.isEmpty()) return;
    double maxV = 1.0;
    for (const auto& d : data_) maxV = qMax(maxV, d.second);
    const int n = data_.size();
    const int w = width(), h = height();
    const int barW = qMax(20, (w - 32) / (n * 2));
    for (int i = 0; i < n; ++i) {
        const int x = 16 + i * (barW * 2) + barW / 2;
        const int barH = static_cast<int>((data_[i].second / maxV) * (h - 60));
        painter.setBrush(i % 2 == 0 ? QColor("#FF5A1F") : QColor("#F02D24"));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(x, h - 36 - barH, barW, barH, 4, 4);
        painter.setPen(QColor("#F4F4F5"));
        painter.drawText(QRect(x - 10, h - 36 - barH - 18, barW + 20, 16), Qt::AlignCenter, QString::number(data_[i].second, 'g', 3));
        painter.setPen(QColor("#A1A1AA"));
        painter.drawText(QRect(x - 20, h - 32, barW + 40, 28), Qt::AlignCenter | Qt::TextWordWrap, data_[i].first);
    }
}

// --- Donut chart (QPainter) -------------------------------------------------
DonutWidget::DonutWidget(QWidget* parent) : QWidget(parent) {
    setMinimumSize(180, 180);
}
void DonutWidget::setPercentage(double pct, const QString& label) {
    pct_ = qBound(0.0, pct, 100.0);
    label_ = label;
    update();
}
void DonutWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor("#14161B"));
    const int side = qMin(width(), height()) - 40;
    QRect r((width() - side) / 2, (height() - side) / 2 - 10, side, side);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#2A2D35"));
    painter.drawEllipse(r);
    painter.setBrush(QColor("#FF5A1F"));
    // Qt angles: 16ths of degree, 0 = 3 o'clock, positive = counter-clockwise.
    painter.drawPie(r, 90 * 16, static_cast<int>(-pct_ * 3.6 * 16));
    painter.setBrush(QColor("#14161B"));
    painter.drawEllipse(r.adjusted(side / 4, side / 4, -side / 4, -side / 4));
    painter.setPen(QColor("#F4F4F5"));
    QFont f = painter.font(); f.setBold(true); f.setPointSize(14); painter.setFont(f);
    painter.drawText(r, Qt::AlignCenter, QString::number(pct_, 'f', 1) + "%");
    painter.setPen(QColor("#A1A1AA"));
    QFont f2 = painter.font(); f2.setPointSize(10); f2.setBold(false); painter.setFont(f2);
    painter.drawText(QRect(0, height() - 24, width(), 20), Qt::AlignCenter, label_);
}


AdsLabPage::AdsLabPage(QWidget* parent) : QWidget(parent) {
    // Centered, Chrome-style: header upar, output poori height fill kare.
    auto* outer = new QHBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0); outer->addStretch();
    auto* central = new QWidget(this); central->setMaximumWidth(800); central->setMinimumWidth(360);
    auto* lay = new QVBoxLayout(central);
    lay->setContentsMargins(24, 28, 24, 24);
    lay->setSpacing(14);
    outer->addWidget(central, 1); outer->addStretch();
    auto* pill = new QLabel(QStringLiteral("ADS LAB"));
    pill->setStyleSheet(QStringLiteral("background:#FF5A1F;color:#FFFFFF;border-radius:10px;padding:4px 12px;font-size:11px;font-weight:700;"));
    pill->setFixedHeight(20);
    { auto* pillRow = new QHBoxLayout(); pillRow->addWidget(pill); pillRow->addStretch(); lay->addLayout(pillRow); }
    auto* title = new QLabel(QStringLiteral("ADS Lab"));
    title->setStyleSheet(QStringLiteral("color:#F4F4F5;font-size:26px;font-weight:700;"));
    auto* sub = new QLabel(QStringLiteral("Asli DSA structures ke live results — Trie, Inverted Index, Heap Top-K, Graph BFS, Hash Table, AVL Tree. Koi fake number nahi, sab isi run me measure hua hai."));
    sub->setWordWrap(true);
    sub->setStyleSheet(QStringLiteral("color:#A1A1AA;font-size:13px;"));
    lay->addWidget(title); lay->addWidget(sub);
    out_ = new QPlainTextEdit(this);
    out_->setReadOnly(true);
    out_->setStyleSheet(QStringLiteral("background:#14161B;color:#F4F4F5;border:1px solid #2A2D35;border-radius:12px;padding:12px;font-family:Consolas;font-size:13px;"));
    lay->addWidget(out_, 1);
    // Visualizations row: bar chart (left, expanding) + donut (right, fixed).
    auto* chartsRow = new QHBoxLayout();
    bar_ = new BarChartWidget(this);
    bar_->setStyleSheet(QStringLiteral("background:#14161B;border:1px solid #2A2D35;border-radius:12px;"));
    donut_ = new DonutWidget(this);
    donut_->setFixedSize(220, 200);
    donut_->setStyleSheet(QStringLiteral("background:#14161B;border:1px solid #2A2D35;border-radius:12px;"));
    chartsRow->addWidget(bar_, 1);
    chartsRow->addWidget(donut_);
    lay->addLayout(chartsRow);
    // Button full-width grey bar na bane, isliye HBox me fixed orange button.
    auto* row = new QHBoxLayout();
    row->addStretch();
    auto* btn = new QPushButton(QStringLiteral("Run all ADS demos"), this);
    btn->setFixedSize(220, 40);
    btn->setStyleSheet(QStringLiteral("background:#FF5A1F;color:#FFFFFF;border:none;border-radius:20px;font-weight:700;"));
    connect(btn, &QPushButton::clicked, this, &AdsLabPage::runAll);
    row->addWidget(btn);
    row->addStretch();
    lay->addLayout(row);
    runAll();
}

void AdsLabPage::runAll() {
    out_->clear();
    // Trie
    Trie trie;
    for (const char* w : {"search", "secure", "server", "browser", "bookmark"}) trie.insert(w);
    QString s; for (const auto& x : trie.suggestions("sea", 10)) s += QString::fromStdString(x) + " ";
    out_->appendPlainText(QStringLiteral("Trie 'sea' -> %1").arg(s));
    // Inverted index
    InvertedIndex idx;
    idx.addDocument(1, "corevex browser search engine fast");
    idx.addDocument(2, "corevex search engine local index");
    out_->appendPlainText(QStringLiteral("Index AND 'corevex search' -> %1 docs, phrase 'search engine' -> %2 docs")
        .arg(idx.searchAnd("corevex search").size()).arg(idx.phraseSearch("search engine").size()));
    // Heap Top-K
    auto top = BinaryHeap<std::pair<double, int>>::topK({{1.2, 1}, {4.5, 2}, {9.0, 4}}, 2);
    out_->appendPlainText(QStringLiteral("Heap Top-2 -> doc %1 (score %2)").arg(top[0].second).arg(top[0].first));
    // Graph
    EvidenceGraph g;
    int src = g.addNode(EvidenceGraph::NodeType::Source, "source");
    int claim = g.addNode(EvidenceGraph::NodeType::Claim, "claim");
    g.addEdge(src, claim, EvidenceGraph::EdgeType::Supports);
    out_->appendPlainText(QStringLiteral("Graph BFS -> %1 nodes visited").arg(g.bfs(src).size()));
    // Hash table
    HashTable<std::string, int> h;
    for (int i = 0; i < 100; ++i) h.insert("term" + std::to_string(i), i);
    out_->appendPlainText(QStringLiteral("Hash size=%1 load=%2 collisions=%3")
        .arg(h.size()).arg(h.loadFactor()).arg(h.collisionCount()));
    // AVL
    AvlTree<int, std::string> avl;
    std::vector<int> keys; for (int i = 1; i <= 20; ++i) keys.push_back(i);
    std::mt19937 rng(42); std::shuffle(keys.begin(), keys.end(), rng);
    for (int k : keys) avl.insert(k, "doc");
    out_->appendPlainText(QStringLiteral("AVL size=%1 height=%2 rotations=%3 range[5,10]=%4")
        .arg(avl.size()).arg(avl.height()).arg(avl.rotationCount()).arg(avl.rangeQuery(5, 10).size()));
    // Charts ko usi run ke numbers do (fake nahi).
    if (bar_) {
        QVector<QPair<QString, double>> data;
        data.append({QStringLiteral("Trie"), static_cast<double>(trie.suggestions("sea", 10).size())});
        data.append({QStringLiteral("Index"), static_cast<double>(idx.searchAnd("corevex search").size())});
        data.append({QStringLiteral("Heap"), top[0].first});
        data.append({QStringLiteral("Graph"), static_cast<double>(g.bfs(src).size())});
        data.append({QStringLiteral("Hash"), static_cast<double>(h.collisionCount())});
        data.append({QStringLiteral("AVL"), static_cast<double>(avl.height())});
        bar_->setData(data);
    }
    if (donut_) donut_->setPercentage(h.loadFactor() * 100.0, QStringLiteral("Hash Load"));
}

}  // namespace corevex
