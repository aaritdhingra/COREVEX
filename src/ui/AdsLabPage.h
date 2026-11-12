#pragma once
// ============================================================================
// SECTION 9 — ADS LAB (asli data structures, asli numbers + visualizations)
// ----------------------------------------------------------------------------
// Kya karta hai:
//   src/ads/* ke asli implementations ko chalata hai aur unke asli
//   results dikhata hai: Trie suggestions, inverted-index search +
//   phrase search, heap Top-K, graph BFS, hash-table load/collisions,
//   AVL inorder/height/rotations/range.
// Visualizations (Design System v2.0 jaisa):
//   * Bar chart — har structure ka count/score, QPainter se bana hai,
//     koi extra Qt Charts module nahi chahiye.
//   * Donut chart — Hash Table load factor (used vs free slots).
//   Charts ke numbers bhi usi run ke hain, hard-coded nahi — runAll()
//   me pehle structures chalate hain, phir charts ko wahi values dete hain.
// ============================================================================
#include <QPlainTextEdit>
#include <QVector>
#include <QWidget>

namespace corevex {

class BarChartWidget : public QWidget {
    Q_OBJECT
public:
    explicit BarChartWidget(QWidget* parent = nullptr);
    void setData(const QVector<QPair<QString, double>>& data);
protected:
    void paintEvent(QPaintEvent* event) override;
private:
    QVector<QPair<QString, double>> data_;
};

class DonutWidget : public QWidget {
    Q_OBJECT
public:
    explicit DonutWidget(QWidget* parent = nullptr);
    void setPercentage(double pct, const QString& label);
protected:
    void paintEvent(QPaintEvent* event) override;
private:
    double pct_ = 0.0;
    QString label_;
};

class AdsLabPage : public QWidget {
    Q_OBJECT
public:
    explicit AdsLabPage(QWidget* parent = nullptr);
private slots:
    void runAll();
private:
    QPlainTextEdit* out_ = nullptr;
    BarChartWidget* bar_ = nullptr;
    DonutWidget* donut_ = nullptr;
};

}  // namespace corevex
