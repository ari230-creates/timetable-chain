#ifndef TIMETABLE_CHAIN_DATA_PAGE_HPP
#define TIMETABLE_CHAIN_DATA_PAGE_HPP

#include "core/dataset.hpp"

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QTabWidget>

namespace gui {

class DataPage : public QWidget {
    Q_OBJECT

public:
    DataPage(core::Dataset& dataset, QWidget* parent = nullptr);

    bool loadFile(const QString& path);
    void openLoadFileDialog(); // For shortcut Cmd+L

Q_SIGNALS:
    void datasetLoaded();

private Q_SLOTS:
    void onLoadClicked();

private:
    void setupUi();
    void updateViews();

    core::Dataset& m_dataset;

    QPushButton* m_loadBtn;
    QLabel* m_filePathLabel;
    QLabel* m_warningBanner;

    QLabel* m_subjectsCountLabel;
    QLabel* m_facultyCountLabel;
    QLabel* m_slotsCountLabel;
    QLabel* m_periodsNeededLabel;

    QTableWidget* m_subjectsTable;
    QTableWidget* m_facultyTable;
    QTableWidget* m_slotsTable;

protected:
    void paintEvent(QPaintEvent* event) override;
};

} // namespace gui

#endif // TIMETABLE_CHAIN_DATA_PAGE_HPP
