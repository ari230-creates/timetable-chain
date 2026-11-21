#ifndef TIMETABLE_CHAIN_SCHEDULE_GRID_HPP
#define TIMETABLE_CHAIN_SCHEDULE_GRID_HPP

#include "core/timetable.hpp"
#include "core/dataset.hpp"

#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <map>

namespace gui {

class ScheduleGrid : public QWidget {
    Q_OBJECT

public:
    explicit ScheduleGrid(bool interactive = false, QWidget* parent = nullptr);

    /// Updates the grid display with the current timetable and dataset.
    void updateSchedule(const core::Timetable& timetable, const core::Dataset& dataset);

    /// Exports current schedule to CSV.
    bool exportToCsv(const QString& path);

    /// Exports current schedule to PDF.
    bool exportToPdf(const QString& path);

    /// Returns currently displayed timetable.
    const core::Timetable& getTimetable() const { return m_timetable; }

    /// Sets whether right-click and clicks trigger interactive editing.
    void setInteractive(bool interactive);

Q_SIGNALS:
    void cellSelected(int slotId);
    void cellRemoveRequested(int slotId);

private Q_SLOTS:
    void onCellClicked(int row, int col);
    void showContextMenu(const QPoint& pos);
    void onExportCsvClicked();
    void onExportPdfClicked();

private:
    void setupUi();
    void rebuildGridStructure();

    bool m_interactive = false;
    core::Timetable m_timetable;
    core::Dataset m_dataset;

    QLabel* m_headerLabel;
    QPushButton* m_exportCsvBtn;
    QPushButton* m_exportPdfBtn;
    QTableWidget* m_table;

    // Grid mapping: row -> day string, col -> period number
    std::vector<std::string> m_days;
    int m_maxPeriod = 0;
    // Map (day, period) -> slotId
    std::map<std::pair<std::string, int>, int> m_dayPeriodToSlot;
    // Map slotId -> (row, col)
    std::map<int, std::pair<int, int>> m_slotToRowCol;
};

} // namespace gui

#endif // TIMETABLE_CHAIN_SCHEDULE_GRID_HPP
