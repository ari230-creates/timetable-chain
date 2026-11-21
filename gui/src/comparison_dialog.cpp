#include "comparison_dialog.hpp"
#include "theme.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QPainter>
#include <QElapsedTimer>

namespace gui {

StrategyChartView::StrategyChartView(QWidget* parent)
    : QWidget(parent) {
    setMinimumSize(480, 240);
}

void StrategyChartView::setResults(const std::vector<StrategyResult>& results) {
    m_results = results;
    update();
}

void StrategyChartView::paintEvent(QPaintEvent* /*event*/) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    p.fillRect(rect(), Theme::cardColor());

    if (m_results.empty()) return;

    int maxVal = 1;
    for (const auto& r : m_results) {
        if (r.placements > maxVal) maxVal = r.placements;
    }

    int leftMargin = 60;
    int rightMargin = 30;
    int topMargin = 40;
    int bottomMargin = 50;

    int chartWidth = width() - leftMargin - rightMargin;
    int chartHeight = height() - topMargin - bottomMargin;

    // Draw axes
    p.setPen(QPen(Theme::borderColor(), 1));
    p.drawLine(leftMargin, topMargin + chartHeight, leftMargin + chartWidth, topMargin + chartHeight);
    p.drawLine(leftMargin, topMargin, leftMargin, topMargin + chartHeight);

    // Grid lines
    int numTicks = 4;
    p.setFont(QFont("Helvetica", 9));
    for (int i = 0; i <= numTicks; ++i) {
        int y = topMargin + chartHeight - (i * chartHeight / numTicks);
        int val = i * maxVal / numTicks;

        p.setPen(QPen(Theme::borderColor(), 1, Qt::DotLine));
        p.drawLine(leftMargin, y, leftMargin + chartWidth, y);

        p.setPen(Theme::mutedTextColor());
        p.drawText(QRect(10, y - 10, leftMargin - 15, 20), Qt::AlignRight | Qt::AlignVCenter, QString::number(val));
    }

    // Colors for modes
    QVector<QColor> barColors = {
        QColor("#5B5BD6"), // Plain
        QColor("#2E9E6B"), // MRV
        QColor("#E0A030")  // Best
    };

    int n = static_cast<int>(m_results.size());
    int groupWidth = chartWidth / n;
    int barWidth = std::min(45, groupWidth - 30);

    for (int i = 0; i < n; ++i) {
        const auto& res = m_results[i];
        int barH = (static_cast<qint64>(res.placements) * chartHeight) / maxVal;
        int x = leftMargin + i * groupWidth + (groupWidth - barWidth) / 2;
        int y = topMargin + chartHeight - barH;

        QRect barRect(x, y, barWidth, barH);
        p.setPen(Qt::NoPen);
        p.setBrush(barColors[i % barColors.size()]);
        p.drawRoundedRect(barRect, 6, 6);

        // Draw value on top of bar
        p.setPen(Theme::textColor());
        p.setFont(QFont("Helvetica", 10, QFont::Bold));
        p.drawText(QRect(x - 20, y - 24, barWidth + 40, 20), Qt::AlignCenter, QString::number(res.placements));

        // Draw label below bar
        p.setPen(Theme::mutedTextColor());
        p.setFont(QFont("Helvetica", 10, QFont::DemiBold));
        p.drawText(QRect(x - 25, topMargin + chartHeight + 10, barWidth + 50, 30), Qt::AlignHCenter | Qt::AlignTop, res.name);
    }

    // Chart Title
    p.setPen(Theme::textColor());
    p.setFont(QFont("Helvetica", 11, QFont::Bold));
    p.drawText(QRect(leftMargin, 10, chartWidth, 25), Qt::AlignLeft | Qt::AlignVCenter, "Placements Explored by Search Strategy");
}

ComparisonDialog::ComparisonDialog(const core::Dataset& dataset, QWidget* parent)
    : QDialog(parent), m_dataset(dataset) {
    setWindowTitle("Strategy Benchmark & Comparison");
    resize(620, 520);
    setModal(true);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel("Constraint Satisfaction Search Benchmarks", this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: 700; color: #5B5BD6;");
    mainLayout->addWidget(titleLabel);

    m_chartView = new StrategyChartView(this);
    m_chartView->setGraphicsEffect(Theme::createCardShadow(m_chartView));
    mainLayout->addWidget(m_chartView);

    runComparison();

    auto* closeBtn = new QPushButton("Close", this);
    closeBtn->setProperty("class", "primary-btn");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(closeBtn, 0, Qt::AlignRight);
}

void ComparisonDialog::runComparison() {
    std::vector<StrategyResult> results;

    auto testMode = [&](core::GeneratorMode mode, const QString& name) {
        core::Generator gen(m_dataset);
        core::Timetable tt;
        QElapsedTimer timer;
        timer.start();
        gen.run(mode, tt);
        qint64 elapsed = timer.elapsed();

        StrategyResult r;
        r.name = name;
        r.placements = gen.getStats().placements;
        r.backtracks = gen.getStats().backtracks;
        r.score = gen.getStats().score;
        r.elapsedMs = elapsed;
        results.push_back(r);
    };

    testMode(core::GeneratorMode::Plain, "Plain");
    testMode(core::GeneratorMode::MRV, "MRV");
    testMode(core::GeneratorMode::Best, "Best (Top 50)");

    m_chartView->setResults(results);

    // Table below chart
    auto* table = new QTableWidget(this);
    table->setRowCount(static_cast<int>(results.size()));
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"Strategy", "Placements", "Backtracks", "Penalty Score", "Time (ms)"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setFixedHeight(125);

    for (size_t i = 0; i < results.size(); ++i) {
        const auto& r = results[i];
        table->setItem(static_cast<int>(i), 0, new QTableWidgetItem(r.name));
        table->setItem(static_cast<int>(i), 1, new QTableWidgetItem(QString::number(r.placements)));
        table->setItem(static_cast<int>(i), 2, new QTableWidgetItem(QString::number(r.backtracks)));
        table->setItem(static_cast<int>(i), 3, new QTableWidgetItem(QString::number(r.score)));
        table->setItem(static_cast<int>(i), 4, new QTableWidgetItem(QString::number(r.elapsedMs)));
    }

    layout()->addWidget(table);
}

} // namespace gui
