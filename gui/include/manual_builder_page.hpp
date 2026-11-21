#ifndef TIMETABLE_CHAIN_MANUAL_BUILDER_PAGE_HPP
#define TIMETABLE_CHAIN_MANUAL_BUILDER_PAGE_HPP

#include "core/dataset.hpp"
#include "core/blockchain.hpp"
#include "core/timetable.hpp"
#include "schedule_grid.hpp"

#include <QWidget>
#include <QPushButton>
#include <QLabel>

namespace gui {

class ManualBuilderPage : public QWidget {
    Q_OBJECT

public:
    ManualBuilderPage(core::Dataset& dataset, core::Blockchain& blockchain, QWidget* parent = nullptr);

    void onDatasetLoaded();

Q_SIGNALS:
    void blockAdded();

private Q_SLOTS:
    void onCellSelected(int slotId);
    void onCellRemoveRequested(int slotId);
    void onClearClicked();
    void onValidateClicked();
    void onCommitClicked();

private:
    void setupUi();
    void updateCommitButtonState();

    core::Dataset& m_dataset;
    core::Blockchain& m_blockchain;
    core::Timetable m_timetable;

    ScheduleGrid* m_grid;
    QPushButton* m_clearBtn;
    QPushButton* m_validateBtn;
    QPushButton* m_commitBtn;
    QLabel* m_statusBanner;

protected:
    void paintEvent(QPaintEvent* event) override;
};

} // namespace gui

#endif // TIMETABLE_CHAIN_MANUAL_BUILDER_PAGE_HPP
