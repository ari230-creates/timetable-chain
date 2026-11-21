#ifndef TIMETABLE_CHAIN_BLOCKCHAIN_PAGE_HPP
#define TIMETABLE_CHAIN_BLOCKCHAIN_PAGE_HPP

#include "core/blockchain.hpp"
#include "core/dataset.hpp"

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

namespace gui {

class BlockchainPage : public QWidget {
    Q_OBJECT

public:
    BlockchainPage(core::Blockchain& blockchain, const core::Dataset& dataset, QWidget* parent = nullptr);

    void refreshBlocks();
    void verifyChain(); // Triggered by button or Cmd+Shift+V

Q_SIGNALS:
    void chainStatusUpdated(int blockCount, bool verified);

private Q_SLOTS:
    void onVerifyClicked();
    void onSaveClicked();
    void onLoadClicked();
    void onTamperClicked();
    void onBlockInspectClicked(int blockIndex);

private:
    void setupUi();

    core::Blockchain& m_blockchain;
    const core::Dataset& m_dataset;

    QPushButton* m_verifyBtn;
    QPushButton* m_saveBtn;
    QPushButton* m_loadBtn;
    QPushButton* m_tamperBtn;

    QLabel* m_resultBanner;
    QScrollArea* m_scrollArea;
    QWidget* m_cardsContainer;
    QVBoxLayout* m_cardsLayout;

    core::VerificationReport m_lastReport;
    bool m_hasVerified = false;

protected:
    void paintEvent(QPaintEvent* event) override;
};

} // namespace gui

#endif // TIMETABLE_CHAIN_BLOCKCHAIN_PAGE_HPP
