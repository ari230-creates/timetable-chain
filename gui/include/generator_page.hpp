#ifndef TIMETABLE_CHAIN_GENERATOR_PAGE_HPP
#define TIMETABLE_CHAIN_GENERATOR_PAGE_HPP

#include "core/dataset.hpp"
#include "core/blockchain.hpp"
#include "core/generator.hpp"
#include "schedule_grid.hpp"

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QThread>

namespace gui {

class GeneratorWorker : public QThread {
    Q_OBJECT

public:
    GeneratorWorker(const core::Dataset& dataset,
                    core::GeneratorMode mode,
                    bool mineAfter,
                    core::Blockchain* blockchain,
                    QObject* parent = nullptr);

    void run() override;

Q_SIGNALS:
    void finished(bool success,
                  const core::Timetable& timetable,
                  const core::GeneratorStats& stats,
                  bool mined);

private:
    core::Dataset m_dataset;
    core::GeneratorMode m_mode;
    bool m_mineAfter;
    core::Blockchain* m_blockchain;
};

class GeneratorPage : public QWidget {
    Q_OBJECT

public:
    GeneratorPage(core::Dataset& dataset, core::Blockchain& blockchain, QWidget* parent = nullptr);

    void onDatasetLoaded();
    void startGeneration(); // For shortcut Cmd+G

Q_SIGNALS:
    void blockAdded();

private Q_SLOTS:
    void onGenerateClicked();
    void onGenerateAndMineClicked();
    void onCompareClicked();
    void onWorkerFinished(bool success,
                          const core::Timetable& timetable,
                          const core::GeneratorStats& stats,
                          bool mined);

private:
    void setupUi();
    void animateStat(QLabel* label, int startVal, int endVal);

    core::Dataset& m_dataset;
    core::Blockchain& m_blockchain;

    QComboBox* m_strategyCombo;
    QPushButton* m_generateBtn;
    QPushButton* m_generateMineBtn;
    QPushButton* m_compareBtn;
    QProgressBar* m_busyBar;

    QLabel* m_placementsLabel;
    QLabel* m_backtracksLabel;
    QLabel* m_scoreLabel;
    QLabel* m_solutionsComparedLabel;

    ScheduleGrid* m_grid;

    int m_lastPlacements = 0;
    int m_lastBacktracks = 0;
    int m_lastScore = 0;

protected:
    void paintEvent(QPaintEvent* event) override;
};

} // namespace gui

#endif // TIMETABLE_CHAIN_GENERATOR_PAGE_HPP
