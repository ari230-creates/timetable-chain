#include "slot_dialog.hpp"
#include "core/constraint_checker.hpp"
#include "theme.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>

namespace gui {

SlotDialog::SlotDialog(int slotId,
                       const core::Timetable& timetable,
                       const core::Dataset& dataset,
                       QWidget* parent)
    : QDialog(parent), m_slotId(slotId), m_timetable(timetable), m_dataset(dataset) {
    setWindowTitle("Assign Schedule Slot");
    setModal(true);
    setMinimumWidth(420);
    setupUi();
}

void SlotDialog::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    const auto* slot = m_dataset.findSlot(m_slotId);
    QString slotText = slot ? QString("Slot %1 (%2)").arg(slot->id).arg(QString::fromStdString(slot->toString()))
                            : QString("Slot %1").arg(m_slotId);

    m_slotInfoLabel = new QLabel(QString("Configuring Assignment for %1").arg(slotText), this);
    m_slotInfoLabel->setStyleSheet("font-size: 15px; font-weight: 700; color: #5B5BD6;");
    mainLayout->addWidget(m_slotInfoLabel);

    auto* form = new QFormLayout();
    form->setSpacing(12);

    m_subjectCombo = new QComboBox(this);
    for (const auto& sub : m_dataset.getSubjects()) {
        m_subjectCombo->addItem(QString::fromStdString(sub.name), sub.id);
    }
    form->addRow("Subject:", m_subjectCombo);

    m_facultyCombo = new QComboBox(this);
    for (const auto& fac : m_dataset.getFaculty()) {
        m_facultyCombo->addItem(QString::fromStdString(fac.name), fac.id);
    }
    form->addRow("Faculty:", m_facultyCombo);

    mainLayout->addLayout(form);

    // Validation label
    m_validationLabel = new QLabel(this);
    m_validationLabel->setWordWrap(true);
    m_validationLabel->setStyleSheet("font-size: 12px; font-weight: 600; padding: 6px;");
    mainLayout->addWidget(m_validationLabel);

    // Buttons
    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    m_cancelBtn = new QPushButton("Cancel", this);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(m_cancelBtn);

    m_okBtn = new QPushButton("Assign Slot", this);
    m_okBtn->setProperty("class", "primary-btn");
    connect(m_okBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(m_okBtn);

    mainLayout->addLayout(btnLayout);

    connect(m_subjectCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SlotDialog::onSelectionChanged);
    connect(m_facultyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SlotDialog::onSelectionChanged);

    // If an entry already exists for this slot, pre-select it
    const auto* existing = m_timetable.findEntryBySlot(m_slotId);
    if (existing) {
        int subIdx = m_subjectCombo->findData(existing->subjectId);
        if (subIdx != -1) m_subjectCombo->setCurrentIndex(subIdx);
        int facIdx = m_facultyCombo->findData(existing->facultyId);
        if (facIdx != -1) m_facultyCombo->setCurrentIndex(facIdx);
    }

    updateValidation();
}

int SlotDialog::getSelectedSubjectId() const {
    return m_subjectCombo->currentData().toInt();
}

int SlotDialog::getSelectedFacultyId() const {
    return m_facultyCombo->currentData().toInt();
}

void SlotDialog::onSelectionChanged() {
    updateValidation();
}

void SlotDialog::updateValidation() {
    int subId = getSelectedSubjectId();
    int facId = getSelectedFacultyId();

    const auto* sub = m_dataset.findSubject(subId);
    const auto* fac = m_dataset.findFaculty(facId);
    const auto* slot = m_dataset.findSlot(m_slotId);

    if (!sub || !fac || !slot) {
        m_validationLabel->setText("Invalid selection");
        m_validationLabel->setStyleSheet("color: #D64545; font-weight: 600;");
        m_okBtn->setEnabled(false);
        return;
    }

    // For checking if this slot can be placed:
    // If the slot is currently occupied by the entry we are editing, create a temporary timetable
    // without this slot so rule 3/4 doesn't conflict with itself.
    core::Timetable testTimetable = m_timetable;
    testTimetable.removeEntry(m_slotId);

    std::string reason;
    bool canPlace = core::ConstraintChecker::canPlace(testTimetable, *sub, *fac, *slot, reason);

    if (canPlace) {
        m_validationLabel->setText("✓ Placement satisfies all curricular constraints.");
        m_validationLabel->setStyleSheet("color: #2E9E6B; font-weight: 600; background-color: #E6F4EA; border-radius: 6px; padding: 8px;");
        m_okBtn->setEnabled(true);
    } else {
        m_validationLabel->setText("✕ " + QString::fromStdString(reason));
        m_validationLabel->setStyleSheet("color: #D64545; font-weight: 600; background-color: #FCE8E6; border-radius: 6px; padding: 8px;");
        m_okBtn->setEnabled(false);
    }
}

} // namespace gui
