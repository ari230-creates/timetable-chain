#include "tamper_dialog.hpp"
#include "core/timetable.hpp"
#include "toast.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>

namespace gui {

TamperDialog::TamperDialog(core::Blockchain& blockchain, QWidget* parent)
    : QDialog(parent), m_blockchain(blockchain) {
    setWindowTitle("Blockchain Tamper Demonstration");
    setModal(true);
    setMinimumWidth(460);
    setupUi();
}

void TamperDialog::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel("Simulate Malicious Tampering", this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: 700; color: #D64545;");
    mainLayout->addWidget(titleLabel);

    auto* form = new QFormLayout();
    m_blockCombo = new QComboBox(this);

    // List all blocks except Genesis if possible (or all blocks)
    for (size_t i = 1; i < m_blockchain.size(); ++i) {
        const auto& b = m_blockchain.getBlocks()[i];
        m_blockCombo->addItem(QString("Block #%1 - %2").arg(b.index).arg(QString::fromStdString(b.note)), b.index);
    }
    if (m_blockCombo->count() == 0 && m_blockchain.size() > 0) {
        // Only genesis available
        m_blockCombo->addItem("Block #0 - Genesis Block", 0);
    }
    form->addRow("Target Block:", m_blockCombo);

    m_remineCheck = new QCheckBox("Also re-mine the block (clever attacker)", this);
    m_remineCheck->setStyleSheet("font-weight: 600; color: #1B1F3B;");
    form->addRow("", m_remineCheck);

    mainLayout->addLayout(form);

    m_infoLabel = new QLabel(
        "<b>Attack Mechanics:</b><br>"
        "• <b>Careless Attacker (Unchecked):</b> Corrupts the timetable data without updating proof-of-work. "
        "Verification will immediately fail on this block due to a <b>Hash Mismatch</b>.<br><br>"
        "• <b>Clever Attacker (Checked):</b> Re-calculates valid proof-of-work (nonce & hash) for the altered block. "
        "However, verification will still fail at the <b>NEXT block</b> due to a <b>Broken Cryptographic Link</b> "
        "(prevHash pointer mismatch).",
        this
    );
    m_infoLabel->setWordWrap(true);
    m_infoLabel->setStyleSheet("background-color: #F8F9FD; border: 1px solid #E4E7F0; border-radius: 8px; padding: 12px; font-size: 12px; color: #6B7194;");
    mainLayout->addWidget(m_infoLabel);

    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    m_cancelBtn = new QPushButton("Cancel", this);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(m_cancelBtn);

    m_executeBtn = new QPushButton("Tamper Block", this);
    m_executeBtn->setProperty("class", "danger-btn");
    connect(m_executeBtn, &QPushButton::clicked, this, &TamperDialog::onExecuteClicked);
    btnLayout->addWidget(m_executeBtn);

    mainLayout->addLayout(btnLayout);
}

int TamperDialog::getSelectedBlockIndex() const {
    return m_blockCombo->currentData().toInt();
}

bool TamperDialog::isRemineRequested() const {
    return m_remineCheck->isChecked();
}

void TamperDialog::onExecuteClicked() {
    int blockIdx = getSelectedBlockIndex();
    bool remine = isRemineRequested();

    if (blockIdx < 0 || blockIdx >= static_cast<int>(m_blockchain.size())) {
        reject();
        return;
    }

    auto& block = m_blockchain.getBlocks()[blockIdx];
    if (block.data == "GENESIS") {
        m_blockchain.tamperBlock(blockIdx, "TAMPERED_GENESIS", remine);
    } else {
        core::Timetable tt = core::Timetable::deserialize(block.data);
        auto entries = tt.getEntries();
        if (!entries.empty()) {
            // Modify one entry's teacher
            entries[0].facultyId = (entries[0].facultyId == 1) ? 2 : 1;
            core::Timetable tamperedTt;
            for (const auto& e : entries) tamperedTt.addEntry(e);
            m_blockchain.tamperBlock(blockIdx, tamperedTt.serialize(), remine);
        } else {
            m_blockchain.tamperBlock(blockIdx, "1,2,1;", remine);
        }
    }

    accept();
}

} // namespace gui
