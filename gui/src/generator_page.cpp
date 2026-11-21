#include "generator_page.hpp"
#include "comparison_dialog.hpp"
#include "theme.hpp"
#include "toast.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QVariantAnimation>
#include <QPainter>
#include <QStyleOption>

namespace gui {

GeneratorWorker::GeneratorWorker(const core::Dataset& dataset,
                                 core::GeneratorMode mode,
                                 bool mineAfter,
                                 core::Blockchain* blockchain,
                                 QObject* parent)
    : QThread(parent), m_dataset(dataset), m_mode(mode),
      m_mineAfter(mineAfter), m_blockchain(blockchain) {}

void GeneratorWorker::run() {
    core::Generator gen(m_dataset);
    core::Timetable tt;
    bool ok = gen.run(m_mode, tt);

    bool mined = false;
    if (ok && m_mineAfter && m_blockchain) {
        std::string modeName = (m_mode == core::GeneratorMode::Plain) ? "Plain" :
                               (m_mode == core::GeneratorMode::MRV) ? "MRV" : "Best";
        std::string note = "Generated Schedule (" + modeName + ", Score: " +
                           std::to_string(gen.getStats().score) + ")";
        mined = m_blockchain->addBlock(note, tt.serialize(), m_dataset);
    }

    Q_EMIT finished(ok, tt, gen.getStats(), mined);
}

GeneratorPage::GeneratorPage(core::Dataset& dataset, core::Blockchain& blockchain, QWidget* parent)
    : QWidget(parent), m_dataset(dataset), m_blockchain(blockchain) {
    setupUi();
}

void GeneratorPage::setupUi() {
    setObjectName("GeneratorPage");
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(16);

    // Controls Card
    auto* controlCard = new QFrame(this);
    controlCard->setProperty("class", "card");
    controlCard->setGraphicsEffect(Theme::createCardShadow(controlCard));
    auto* controlLayout = new QHBoxLayout(controlCard);
    controlLayout->setContentsMargins(16, 14, 16, 14);
    controlLayout->setSpacing(12);

    auto* stratLabel = new QLabel("Search Strategy:", controlCard);
    stratLabel->setStyleSheet("font-weight: 600; color: #1B1F3B;");
    controlLayout->addWidget(stratLabel);

    m_strategyCombo = new QComboBox(controlCard);
    m_strategyCombo->addItem("Plain (Data Order)", static_cast<int>(core::GeneratorMode::Plain));
    m_strategyCombo->addItem("MRV (Most Constrained First)", static_cast<int>(core::GeneratorMode::MRV));
    m_strategyCombo->addItem("Best (Explore 50 Solutions)", static_cast<int>(core::GeneratorMode::Best));
    m_strategyCombo->setCurrentIndex(1); // Default to MRV
    controlLayout->addWidget(m_strategyCombo);

    controlLayout->addSpacing(8);

    m_generateBtn = new QPushButton("Generate", controlCard);
    m_generateBtn->setProperty("class", "primary-btn");
    connect(m_generateBtn, &QPushButton::clicked, this, &GeneratorPage::onGenerateClicked);
    controlLayout->addWidget(m_generateBtn);

    m_generateMineBtn = new QPushButton("Generate & Mine Block", controlCard);
    m_generateMineBtn->setProperty("class", "primary-btn");
    connect(m_generateMineBtn, &QPushButton::clicked, this, &GeneratorPage::onGenerateAndMineClicked);
    controlLayout->addWidget(m_generateMineBtn);

    m_compareBtn = new QPushButton("Compare Strategies", controlCard);
    m_compareBtn->setProperty("class", "secondary-btn");
    connect(m_compareBtn, &QPushButton::clicked, this, &GeneratorPage::onCompareClicked);
    controlLayout->addWidget(m_compareBtn);

    controlLayout->addStretch();

    m_busyBar = new QProgressBar(controlCard);
    m_busyBar->setRange(0, 0); // Indeterminate busy indicator
    m_busyBar->setFixedWidth(120);
    m_busyBar->setFixedHeight(12);
    m_busyBar->setTextVisible(false);
    m_busyBar->hide();
    controlLayout->addWidget(m_busyBar);

    mainLayout->addWidget(controlCard);

    // Three Stat Cards
    auto* statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(16);

    auto makeStatCard = [this](const QString& title, QLabel*& valLabel, const QString& subtext) {
        auto* card = new QFrame(this);
        card->setProperty("class", "card");
        card->setGraphicsEffect(Theme::createCardShadow(card));
        auto* l = new QVBoxLayout(card);
        l->setContentsMargins(16, 14, 16, 14);
        l->setSpacing(4);

        auto* t = new QLabel(title, card);
        t->setProperty("class", "stat-title");
        l->addWidget(t);

        valLabel = new QLabel("0", card);
        valLabel->setProperty("class", "stat-value");
        l->addWidget(valLabel);

        auto* s = new QLabel(subtext, card);
        s->setProperty("class", "stat-sub");
        l->addWidget(s);

        return card;
    };

    statsLayout->addWidget(makeStatCard("PLACEMENTS", m_placementsLabel, "Recursive search choices made"));
    statsLayout->addWidget(makeStatCard("BACKTRACKS", m_backtracksLabel, "Unproductive branches pruned"));

    // Quality card with dynamic solutions compared label
    auto* scoreCard = new QFrame(this);
    scoreCard->setProperty("class", "card");
    scoreCard->setGraphicsEffect(Theme::createCardShadow(scoreCard));
    auto* scoreLayout = new QVBoxLayout(scoreCard);
    scoreLayout->setContentsMargins(16, 14, 16, 14);
    scoreLayout->setSpacing(4);

    auto* scoreTitle = new QLabel("QUALITY SCORE", scoreCard);
    scoreTitle->setProperty("class", "stat-title");
    scoreLayout->addWidget(scoreTitle);

    m_scoreLabel = new QLabel("0", scoreCard);
    m_scoreLabel->setProperty("class", "stat-value");
    scoreLayout->addWidget(m_scoreLabel);

    m_solutionsComparedLabel = new QLabel("Lower penalty score is better", scoreCard);
    m_solutionsComparedLabel->setProperty("class", "stat-sub");
    scoreLayout->addWidget(m_solutionsComparedLabel);

    statsLayout->addWidget(scoreCard);
    mainLayout->addLayout(statsLayout);

    // Schedule Grid
    m_grid = new ScheduleGrid(false, this);
    mainLayout->addWidget(m_grid, 1);
}

void GeneratorPage::onDatasetLoaded() {
    m_grid->updateSchedule(core::Timetable(), m_dataset);
}

void GeneratorPage::startGeneration() {
    onGenerateClicked();
}

void GeneratorPage::onGenerateClicked() {
    if (m_dataset.getSubjects().empty()) {
        Toast::show(this, "Please load a curriculum dataset first.", ToastType::Error);
        return;
    }

    core::GeneratorMode mode = static_cast<core::GeneratorMode>(m_strategyCombo->currentData().toInt());

    m_generateBtn->setEnabled(false);
    m_generateMineBtn->setEnabled(false);
    m_busyBar->show();

    auto* worker = new GeneratorWorker(m_dataset, mode, false, nullptr, this);
    connect(worker, &GeneratorWorker::finished, this, &GeneratorPage::onWorkerFinished);
    connect(worker, &GeneratorWorker::finished, worker, &QObject::deleteLater);
    worker->start();
}

void GeneratorPage::onGenerateAndMineClicked() {
    if (m_dataset.getSubjects().empty()) {
        Toast::show(this, "Please load a curriculum dataset first.", ToastType::Error);
        return;
    }

    core::GeneratorMode mode = static_cast<core::GeneratorMode>(m_strategyCombo->currentData().toInt());

    m_generateBtn->setEnabled(false);
    m_generateMineBtn->setEnabled(false);
    m_busyBar->show();

    auto* worker = new GeneratorWorker(m_dataset, mode, true, &m_blockchain, this);
    connect(worker, &GeneratorWorker::finished, this, &GeneratorPage::onWorkerFinished);
    connect(worker, &GeneratorWorker::finished, worker, &QObject::deleteLater);
    worker->start();
}

void GeneratorPage::onCompareClicked() {
    if (m_dataset.getSubjects().empty()) {
        Toast::show(this, "Please load a curriculum dataset first.", ToastType::Error);
        return;
    }
    ComparisonDialog dlg(m_dataset, this);
    dlg.exec();
}

void GeneratorPage::animateStat(QLabel* label, int startVal, int endVal) {
    auto* anim = new QVariantAnimation(this);
    anim->setDuration(350);
    anim->setStartValue(startVal);
    anim->setEndValue(endVal);
    connect(anim, &QVariantAnimation::valueChanged, label, [label](const QVariant& value) {
        label->setText(QString::number(value.toInt()));
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void GeneratorPage::onWorkerFinished(bool success,
                                     const core::Timetable& timetable,
                                     const core::GeneratorStats& stats,
                                     bool mined) {
    m_generateBtn->setEnabled(true);
    m_generateMineBtn->setEnabled(true);
    m_busyBar->hide();

    if (!success) {
        Toast::show(this, "No valid timetable could be found with the given constraints.", ToastType::Error);
        return;
    }

    animateStat(m_placementsLabel, m_lastPlacements, stats.placements);
    animateStat(m_backtracksLabel, m_lastBacktracks, stats.backtracks);
    animateStat(m_scoreLabel, m_lastScore, stats.score);

    m_lastPlacements = stats.placements;
    m_lastBacktracks = stats.backtracks;
    m_lastScore = stats.score;

    if (m_strategyCombo->currentData().toInt() == static_cast<int>(core::GeneratorMode::Best)) {
        m_solutionsComparedLabel->setText(QString("%1 solutions compared (lowest penalty)").arg(stats.solutionsFound));
    } else {
        m_solutionsComparedLabel->setText("Lower penalty score is better");
    }

    m_grid->updateSchedule(timetable, m_dataset);

    if (mined) {
        Toast::show(this, "Schedule generated & mined into Blockchain block successfully!", ToastType::Success);
        Q_EMIT blockAdded();
    } else {
        Toast::show(this, "Timetable generated successfully!", ToastType::Success);
    }
}

void GeneratorPage::paintEvent(QPaintEvent* event) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(event);
}

} // namespace gui
