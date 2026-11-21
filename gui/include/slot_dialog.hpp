#ifndef TIMETABLE_CHAIN_SLOT_DIALOG_HPP
#define TIMETABLE_CHAIN_SLOT_DIALOG_HPP

#include "core/timetable.hpp"
#include "core/dataset.hpp"

#include <QDialog>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>

namespace gui {

class SlotDialog : public QDialog {
    Q_OBJECT

public:
    SlotDialog(int slotId,
               const core::Timetable& timetable,
               const core::Dataset& dataset,
               QWidget* parent = nullptr);

    int getSelectedSubjectId() const;
    int getSelectedFacultyId() const;

private Q_SLOTS:
    void onSelectionChanged();

private:
    void setupUi();
    void updateValidation();

    int m_slotId;
    core::Timetable m_timetable;
    core::Dataset m_dataset;

    QComboBox* m_subjectCombo;
    QComboBox* m_facultyCombo;
    QLabel* m_slotInfoLabel;
    QLabel* m_validationLabel;
    QPushButton* m_okBtn;
    QPushButton* m_cancelBtn;
};

} // namespace gui

#endif // TIMETABLE_CHAIN_SLOT_DIALOG_HPP
