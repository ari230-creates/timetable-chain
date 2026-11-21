#ifndef TIMETABLE_CHAIN_TAMPER_DIALOG_HPP
#define TIMETABLE_CHAIN_TAMPER_DIALOG_HPP

#include "core/blockchain.hpp"

#include <QDialog>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>

namespace gui {

class TamperDialog : public QDialog {
    Q_OBJECT

public:
    TamperDialog(core::Blockchain& blockchain, QWidget* parent = nullptr);

    int getSelectedBlockIndex() const;
    bool isRemineRequested() const;

private Q_SLOTS:
    void onExecuteClicked();

private:
    void setupUi();

    core::Blockchain& m_blockchain;
    QComboBox* m_blockCombo;
    QCheckBox* m_remineCheck;
    QLabel* m_infoLabel;
    QPushButton* m_executeBtn;
    QPushButton* m_cancelBtn;
};

} // namespace gui

#endif // TIMETABLE_CHAIN_TAMPER_DIALOG_HPP
