#ifndef TIMETABLE_CHAIN_BLOCK_DETAIL_DIALOG_HPP
#define TIMETABLE_CHAIN_BLOCK_DETAIL_DIALOG_HPP

#include "core/block.hpp"
#include "core/dataset.hpp"
#include "schedule_grid.hpp"

#include <QDialog>
#include <QLabel>
#include <QPushButton>

namespace gui {

class BlockDetailDialog : public QDialog {
    Q_OBJECT

public:
    BlockDetailDialog(const core::Block& block, const core::Dataset& dataset, QWidget* parent = nullptr);

private:
    void setupUi();

    core::Block m_block;
    core::Dataset m_dataset;
    ScheduleGrid* m_grid;
};

} // namespace gui

#endif // TIMETABLE_CHAIN_BLOCK_DETAIL_DIALOG_HPP
