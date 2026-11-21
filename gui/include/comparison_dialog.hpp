#ifndef TIMETABLE_CHAIN_COMPARISON_DIALOG_HPP
#define TIMETABLE_CHAIN_COMPARISON_DIALOG_HPP

#include "core/dataset.hpp"
#include "core/generator.hpp"

#include <QDialog>
#include <QWidget>
#include <vector>

namespace gui {

struct StrategyResult {
    QString name;
    int placements = 0;
    int backtracks = 0;
    int score = 0;
    qint64 elapsedMs = 0;
};

class StrategyChartView : public QWidget {
    Q_OBJECT

public:
    explicit StrategyChartView(QWidget* parent = nullptr);
    void setResults(const std::vector<StrategyResult>& results);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    std::vector<StrategyResult> m_results;
};

class ComparisonDialog : public QDialog {
    Q_OBJECT

public:
    ComparisonDialog(const core::Dataset& dataset, QWidget* parent = nullptr);

private:
    void runComparison();

    core::Dataset m_dataset;
    StrategyChartView* m_chartView;
};

} // namespace gui

#endif // TIMETABLE_CHAIN_COMPARISON_DIALOG_HPP
