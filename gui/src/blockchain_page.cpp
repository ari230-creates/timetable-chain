#include "blockchain_page.hpp"
#include "tamper_dialog.hpp"
#include "block_detail_dialog.hpp"
#include "theme.hpp"
#include "toast.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QFileDialog>
#include <QClipboard>
#include <QApplication>
#include <QPainter>
#include <QDateTime>
#include <QStyleOption>

namespace gui {

namespace {

// Visual vertical chain-link connector
class ChainLinkWidget : public QWidget {
public:
    explicit ChainLinkWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setFixedHeight(32);
        setAttribute(Qt::WA_StyledBackground, true);
        setStyleSheet("background: transparent;");
    }
protected:
    void paintEvent(QPaintEvent* /*event*/) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        int cx = width() / 2;
        p.setPen(QPen(Theme::borderColor(), 2, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(cx, 0, cx, 8);
        p.drawLine(cx, 24, cx, 32);

        // Chain icon link
        p.setPen(QPen(Theme::accentColor(), 2));
        p.setBrush(Theme::cardColor());
        p.drawRoundedRect(cx - 8, 8, 16, 16, 4, 4);
    }
};

// Copyable monospace chip
class HashChip : public QLabel {
public:
    HashChip(const QString& label, const QString& fullHash, QWidget* parent = nullptr)
        : QLabel(parent), m_fullHash(fullHash) {
        QString shortHash = fullHash;
        if (fullHash.size() > 16) {
            shortHash = fullHash.left(10) + "..." + fullHash.right(6);
        }
        setText(QString("<b>%1:</b> <span style=\"font-family: Menlo, 'Courier New', Courier;\">%2</span>").arg(label).arg(shortHash));
        setToolTip(QString("Full Hash: %1\n(Click to copy to clipboard)").arg(fullHash));
        setCursor(Qt::PointingHandCursor);
        setStyleSheet("background-color: #EEF0FD; color: #1B1F3B; font-size: 11px; padding: 4px 8px; border-radius: 6px; border: 1px solid #D6D8F8;");
    }

protected:
    void mousePressEvent(QMouseEvent* /*event*/) override {
        QApplication::clipboard()->setText(m_fullHash);
        Toast::show(window(), "Hash copied to clipboard", ToastType::Info, 1800);
    }

private:
    QString m_fullHash;
};

} // anonymous namespace

BlockchainPage::BlockchainPage(core::Blockchain& blockchain,
                               const core::Dataset& dataset,
                               QWidget* parent)
    : QWidget(parent), m_blockchain(blockchain), m_dataset(dataset) {
    setupUi();
    refreshBlocks();
}

void BlockchainPage::setupUi() {
    setObjectName("BlockchainPage");
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

    m_verifyBtn = new QPushButton("Verify Blockchain", toolbarCard);
    m_verifyBtn->setProperty("class", "primary-btn");
    connect(m_verifyBtn, &QPushButton::clicked, this, &BlockchainPage::onVerifyClicked);
    toolbarLayout->addWidget(m_verifyBtn);

    m_tamperBtn = new QPushButton("Tamper Demo", toolbarCard);
    m_tamperBtn->setProperty("class", "danger-btn");
    connect(m_tamperBtn, &QPushButton::clicked, this, &BlockchainPage::onTamperClicked);
    toolbarLayout->addWidget(m_tamperBtn);

    toolbarLayout->addStretch();

    m_saveBtn = new QPushButton("Save Chain", toolbarCard);
    connect(m_saveBtn, &QPushButton::clicked, this, &BlockchainPage::onSaveClicked);
    toolbarLayout->addWidget(m_saveBtn);

    m_loadBtn = new QPushButton("Load Chain", toolbarCard);
    connect(m_loadBtn, &QPushButton::clicked, this, &BlockchainPage::onLoadClicked);
    toolbarLayout->addWidget(m_loadBtn);

    mainLayout->addWidget(toolbarCard);

    // Result Banner
    m_resultBanner = new QLabel(this);
    m_resultBanner->setWordWrap(true);
    m_resultBanner->hide();
    mainLayout->addWidget(m_resultBanner);

    // Scroll Area for Block Cards
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setObjectName("BlockScrollArea");
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setAttribute(Qt::WA_StyledBackground, true);
    m_scrollArea->viewport()->setAttribute(Qt::WA_StyledBackground, true);

    m_cardsContainer = new QWidget(m_scrollArea);
    m_cardsContainer->setObjectName("BlockCardsContainer");
    m_cardsContainer->setAttribute(Qt::WA_StyledBackground, true);
    m_cardsLayout = new QVBoxLayout(m_cardsContainer);
    m_cardsLayout->setContentsMargins(0, 0, 0, 0);
    m_cardsLayout->setSpacing(0);
    m_cardsLayout->addStretch();

    m_scrollArea->setWidget(m_cardsContainer);
    mainLayout->addWidget(m_scrollArea, 1);
}

void BlockchainPage::paintEvent(QPaintEvent* event) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(event);
}

void BlockchainPage::refreshBlocks() {
    // Clear existing card widgets
    QLayoutItem* item;
    while ((item = m_cardsLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    const auto& blocks = m_blockchain.getBlocks();
    for (size_t i = 0; i < blocks.size(); ++i) {
        const auto& b = blocks[i];

        if (i > 0) {
            m_cardsLayout->addWidget(new ChainLinkWidget(m_cardsContainer));
        }

        auto* card = new QFrame(m_cardsContainer);
        card->setProperty("class", "card");
        card->setGraphicsEffect(Theme::createCardShadow(card));
        auto* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(18, 16, 18, 16);
        cardLayout->setSpacing(10);

        // Header: #index  note  + status badge
        auto* topRow = new QHBoxLayout();
        auto* titleLabel = new QLabel(QString("#%1  %2")
                                          .arg(b.index)
                                          .arg(QString::fromStdString(b.note)), card);
        titleLabel->setStyleSheet("font-size: 16px; font-weight: 700; color: #1B1F3B;");
        topRow->addWidget(titleLabel);

        topRow->addStretch();

        // Status badge
        auto* badge = new QLabel(card);
        if (!m_hasVerified) {
            badge->setText("UNCHECKED");
            badge->setStyleSheet("background-color: #F0F2FA; color: #6B7194; font-weight: 700; font-size: 11px; padding: 4px 10px; border-radius: 6px;");
        } else {
            bool valid = m_lastReport.isBlockValid(b.index);
            if (valid) {
                badge->setText("VERIFIED");
                badge->setStyleSheet("background-color: #E6F4EA; color: #137333; font-weight: 700; font-size: 11px; padding: 4px 10px; border-radius: 6px;");
            } else {
                badge->setText("INVALID");
                badge->setStyleSheet("background-color: #FCE8E6; color: #C5221F; font-weight: 700; font-size: 11px; padding: 4px 10px; border-radius: 6px;");
            }
        }
        topRow->addWidget(badge);
        cardLayout->addLayout(topRow);

        // Metadata row: Timestamp & Nonce
        auto* metaRow = new QHBoxLayout();
        QDateTime dt = QDateTime::fromSecsSinceEpoch(b.timestamp);
        auto* timeLabel = new QLabel(QString("Timestamp: %1 (%2)").arg(dt.toString("yyyy-MM-dd hh:mm:ss")).arg(b.timestamp), card);
        timeLabel->setStyleSheet("font-size: 12px; color: #6B7194;");
        metaRow->addWidget(timeLabel);

        metaRow->addSpacing(16);

        auto* nonceLabel = new QLabel(QString("Nonce: <b>%1</b> (PoW: 3 zeroes)").arg(b.nonce), card);
        nonceLabel->setStyleSheet("font-size: 12px; color: #6B7194;");
        metaRow->addWidget(nonceLabel);

        metaRow->addStretch();

        auto* inspectBtn = new QPushButton("View Schedule", card);
        inspectBtn->setProperty("class", "secondary-btn");
        int bIdx = b.index;
        connect(inspectBtn, &QPushButton::clicked, this, [this, bIdx]() {
            onBlockInspectClicked(bIdx);
        });
        metaRow->addWidget(inspectBtn);

        cardLayout->addLayout(metaRow);

        // Hashes row: PrevHash & Hash
        auto* hashesRow = new QHBoxLayout();
        hashesRow->addWidget(new HashChip("PrevHash", QString::fromStdString(b.prevHash), card));
        hashesRow->addWidget(new HashChip("Hash", QString::fromStdString(b.hash), card));
        hashesRow->addStretch();
        cardLayout->addLayout(hashesRow);

        // If verified and has problems, list them on this card
        if (m_hasVerified && !m_lastReport.isBlockValid(b.index)) {
            auto it = m_lastReport.blockStatuses.find(b.index);
            if (it != m_lastReport.blockStatuses.end()) {
                QString errText;
                for (const auto& prob : it->second.problems) {
                    errText += "• " + QString::fromStdString(prob) + "<br>";
                }
                auto* errLabel = new QLabel(errText, card);
                errLabel->setWordWrap(true);
                errLabel->setStyleSheet("color: #D64545; font-size: 12px; font-weight: 500; background-color: #FFF2F2; padding: 8px; border-radius: 6px; border: 1px solid #FFD4D4;");
                cardLayout->addWidget(errLabel);
            }
        }

        m_cardsLayout->addWidget(card);
    }

    m_cardsLayout->addStretch();
}

void BlockchainPage::verifyChain() {
    onVerifyClicked();
}

void BlockchainPage::onVerifyClicked() {
    m_lastReport = m_blockchain.verify(m_dataset);
    m_hasVerified = true;

    m_resultBanner->show();
    if (m_lastReport.allValid) {
        m_resultBanner->setText("✓ Chain intact and every timetable valid");
        m_resultBanner->setStyleSheet("background-color: #E6F4EA; color: #137333; font-weight: 700; font-size: 14px; padding: 14px; border-radius: 8px; border: 1px solid #CEEAD6;");
        Toast::show(this, "Chain verified: All blocks and cryptographic links valid!", ToastType::Success);
    } else {
        QString text = QString("<b>✕ Verification Failed: %1 Issue(s) Detected</b><br>").arg(m_lastReport.messages.size());
        for (const auto& msg : m_lastReport.messages) {
            text += "• " + QString::fromStdString(msg) + "<br>";
        }
        m_resultBanner->setText(text);
        m_resultBanner->setStyleSheet("background-color: #FCE8E6; color: #C5221F; font-weight: 500; font-size: 13px; padding: 14px; border-radius: 8px; border: 1px solid #FAD2CF;");
        Toast::show(this, "Verification failed! Check issues listed on banner and block cards.", ToastType::Error);
    }

    refreshBlocks();
    Q_EMIT chainStatusUpdated(static_cast<int>(m_blockchain.size()), m_lastReport.allValid);
}

void BlockchainPage::onBlockInspectClicked(int blockIndex) {
    if (blockIndex >= 0 && blockIndex < static_cast<int>(m_blockchain.size())) {
        BlockDetailDialog dlg(m_blockchain.getBlocks()[blockIndex], m_dataset, this);
        dlg.exec();
    }
}

void BlockchainPage::onTamperClicked() {
    TamperDialog dlg(m_blockchain, this);
    if (dlg.exec() == QDialog::Accepted) {
        m_hasVerified = false;
        m_resultBanner->hide();
        refreshBlocks();
        Toast::show(this, "Block tampered in memory! Click 'Verify Blockchain' to test cryptographic detection.", ToastType::Warning, 4500);
    }
}

void BlockchainPage::onSaveClicked() {
    QString path = QFileDialog::getSaveFileName(this, "Save Blockchain Ledger", "blockchain.chain", "Chain Files (*.chain *.txt)");
    if (!path.isEmpty()) {
        std::string err;
        if (m_blockchain.save(path.toStdString(), err)) {
            Toast::show(this, "Blockchain ledger saved successfully", ToastType::Success);
        } else {
            Toast::show(this, QString("Failed to save chain: %1").arg(QString::fromStdString(err)), ToastType::Error);
        }
    }
}

void BlockchainPage::onLoadClicked() {
    QString path = QFileDialog::getOpenFileName(this, "Load Blockchain Ledger", "", "Chain Files (*.chain *.txt)");
    if (!path.isEmpty()) {
        std::string err;
        if (m_blockchain.load(path.toStdString(), err)) {
            m_hasVerified = false;
            m_resultBanner->hide();
            refreshBlocks();
            Toast::show(this, "Blockchain loaded (unverified). Click 'Verify Blockchain' to validate.", ToastType::Info);
            Q_EMIT chainStatusUpdated(static_cast<int>(m_blockchain.size()), false);
        } else {
            Toast::show(this, QString("Failed to load chain: %1").arg(QString::fromStdString(err)), ToastType::Error);
        }
    }
}

} // namespace gui
