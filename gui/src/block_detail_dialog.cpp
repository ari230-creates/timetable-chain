#include "block_detail_dialog.hpp"
#include "theme.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>

namespace gui {

BlockDetailDialog::BlockDetailDialog(const core::Block& block,
                                     const core::Dataset& dataset,
                                     QWidget* parent)
    : QDialog(parent), m_block(block), m_dataset(dataset) {
    setWindowTitle(QString("Block #%1 Schedule Details").arg(m_block.index));
    resize(860, 600);
    setModal(true);
    setupUi();
}

void BlockDetailDialog::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(16);

    auto* headerLayout = new QHBoxLayout();
    auto* titleLabel = new QLabel(QString("Block #%1: %2")
                                     .arg(m_block.index)
                                     .arg(QString::fromStdString(m_block.note)), this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: 700; color: #5B5BD6;");
    headerLayout->addWidget(titleLabel);

    headerLayout->addStretch();

    auto* hashLabel = new QLabel(QString("Hash: %1...%2")
                                     .arg(QString::fromStdString(m_block.hash.substr(0, 10)))
                                     .arg(QString::fromStdString(m_block.hash.substr(m_block.hash.size() - 6))), this);
    hashLabel->setStyleSheet("font-family: Menlo, \"Courier New\", Courier; font-size: 12px; color: #6B7194; background: #EEF0FD; padding: 4px 8px; border-radius: 6px;");
    headerLayout->addWidget(hashLabel);

    mainLayout->addLayout(headerLayout);

    m_grid = new ScheduleGrid(false, this);
    core::Timetable tt = (m_block.data == "GENESIS") ? core::Timetable() : core::Timetable::deserialize(m_block.data);
    m_grid->updateSchedule(tt, m_dataset);
    mainLayout->addWidget(m_grid);

    auto* closeBtn = new QPushButton("Close", this);
    closeBtn->setProperty("class", "primary-btn");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(closeBtn, 0, Qt::AlignRight);
}

} // namespace gui
