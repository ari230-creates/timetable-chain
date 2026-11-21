#include "manual_builder_page.hpp"
#include "slot_dialog.hpp"
#include "core/constraint_checker.hpp"
#include "theme.hpp"
#include "toast.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QInputDialog>
#include <QMessageBox>
#include <QPainter>
#include <QStyleOption>

namespace gui {

ManualBuilderPage::ManualBuilderPage(core::Dataset& dataset, core::Blockchain& blockchain, QWidget* parent)
    : QWidget(parent), m_dataset(dataset), m_blockchain(blockchain) {
    setupUi();
}

void ManualBuilderPage::setupUi() {
    setObjectName("ManualBuilderPage");
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(16);

    // Toolbar Card
    auto* toolbarCard = new QFrame(this);
    toolbarCard->setProperty("class", "card");
    toolbarCard->setGraphicsEffect(Theme::createCardShadow(toolbarCard));
    auto* toolbarLayout = new QHBoxLayout(toolbarCard);
    toolbarLayout->setContentsMargins(16, 14, 16, 14);
    toolbarLayout->setSpacing(12);

    auto* instructions = new QLabel("Click any cell to assign; right-click to remove.", toolbarCard);
    instructions->setStyleSheet("color: #6B7194; font-weight: 500;");
    toolbarLayout->addWidget(instructions);

    toolbarLayout->addStretch();

    m_clearBtn = new QPushButton("Clear All", toolbarCard);
    connect(m_clearBtn, &QPushButton::clicked, this, &ManualBuilderPage::onClearClicked);
    toolbarLayout->addWidget(m_clearBtn);

    m_validateBtn = new QPushButton("Validate Schedule", toolbarCard);
    m_validateBtn->setProperty("class", "secondary-btn");
    connect(m_validateBtn, &QPushButton::clicked, this, &ManualBuilderPage::onValidateClicked);
    toolbarLayout->addWidget(m_validateBtn);

    m_commitBtn = new QPushButton("Commit to Blockchain", toolbarCard);
    m_commitBtn->setProperty("class", "primary-btn");
    m_commitBtn->setEnabled(false);
    connect(m_commitBtn, &QPushButton::clicked, this, &ManualBuilderPage::onCommitClicked);
    toolbarLayout->addWidget(m_commitBtn);

    mainLayout->addWidget(toolbarCard);

    // Validation Status Banner
    m_statusBanner = new QLabel(this);
    m_statusBanner->setWordWrap(true);
    m_statusBanner->hide();
    mainLayout->addWidget(m_statusBanner);

    // Schedule Grid
    m_grid = new ScheduleGrid(true, this);
    connect(m_grid, &ScheduleGrid::cellSelected, this, &ManualBuilderPage::onCellSelected);
    connect(m_grid, &ScheduleGrid::cellRemoveRequested, this, &ManualBuilderPage::onCellRemoveRequested);
    mainLayout->addWidget(m_grid, 1);
}

void ManualBuilderPage::onDatasetLoaded() {
    m_timetable.clear();
    m_grid->updateSchedule(m_timetable, m_dataset);
    updateCommitButtonState();
}

void ManualBuilderPage::onCellSelected(int slotId) {
    SlotDialog dlg(slotId, m_timetable, m_dataset, this);
    if (dlg.exec() == QDialog::Accepted) {
        int subId = dlg.getSelectedSubjectId();
        int facId = dlg.getSelectedFacultyId();

        m_timetable.removeEntry(slotId);
        m_timetable.addEntry({subId, facId, slotId});

        m_grid->updateSchedule(m_timetable, m_dataset);
        updateCommitButtonState();
        m_statusBanner->hide();
    }
}

void ManualBuilderPage::onCellRemoveRequested(int slotId) {
    if (m_timetable.removeEntry(slotId)) {
        m_grid->updateSchedule(m_timetable, m_dataset);
        updateCommitButtonState();
        m_statusBanner->hide();
        Toast::show(this, "Slot assignment removed", ToastType::Info);
    }
}

void ManualBuilderPage::onClearClicked() {
    if (m_timetable.getEntries().empty()) return;
    if (QMessageBox::question(this, "Clear Timetable", "Are you sure you want to clear all manual assignments?") == QMessageBox::Yes) {
        m_timetable.clear();
        m_grid->updateSchedule(m_timetable, m_dataset);
        updateCommitButtonState();
        m_statusBanner->hide();
        Toast::show(this, "Timetable cleared", ToastType::Info);
    }
}

void ManualBuilderPage::onValidateClicked() {
    std::vector<std::string> problems;
    bool valid = core::ConstraintChecker::isValid(m_timetable, m_dataset, problems);

    m_statusBanner->show();
    if (valid) {
        m_statusBanner->setText("✓ All hard constraints satisfied! Every subject has its required periods and no conflicts exist.");
        m_statusBanner->setStyleSheet("background-color: #E6F4EA; color: #137333; font-weight: 600; padding: 12px; border-radius: 8px; border: 1px solid #CEEAD6;");
        Toast::show(this, "Timetable is completely valid!", ToastType::Success);
    } else {
        QString text = "<b>⚠ Schedule Incomplete or Violations Detected:</b><br>";
        for (const auto& p : problems) {
            text += "• " + QString::fromStdString(p) + "<br>";
        }
        m_statusBanner->setText(text);
        m_statusBanner->setStyleSheet("background-color: #FCE8E6; color: #C5221F; font-weight: 500; padding: 12px; border-radius: 8px; border: 1px solid #FAD2CF;");
    }

    updateCommitButtonState();
}

void ManualBuilderPage::updateCommitButtonState() {
    std::vector<std::string> problems;
    bool valid = core::ConstraintChecker::isValid(m_timetable, m_dataset, problems);
    m_commitBtn->setEnabled(valid);
}

void ManualBuilderPage::onCommitClicked() {
    std::vector<std::string> problems;
    if (!core::ConstraintChecker::isValid(m_timetable, m_dataset, problems)) {
        Toast::show(this, "Cannot commit: timetable is not fully valid.", ToastType::Error);
        return;
    }

    bool ok = false;
    QString note = QInputDialog::getText(this, "Commit to Blockchain",
                                         "Enter a note for this verified timetable block:",
                                         QLineEdit::Normal, "Manual Curricular Schedule", &ok);
    if (!ok) return;

    if (note.trimmed().isEmpty()) {
        note = "Manual Timetable";
    }

    std::string err;
    if (m_blockchain.addBlock(note.toStdString(), m_timetable.serialize(), m_dataset, &err)) {
        Toast::show(this, "Successfully mined and committed block to blockchain!", ToastType::Success);
        Q_EMIT blockAdded();
    } else {
        Toast::show(this, QString("Failed to commit: %1").arg(QString::fromStdString(err)), ToastType::Error);
    }
}

void ManualBuilderPage::paintEvent(QPaintEvent* event) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(event);
}

} // namespace gui
