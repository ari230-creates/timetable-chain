#ifndef TIMETABLE_CHAIN_MAINWINDOW_HPP
#define TIMETABLE_CHAIN_MAINWINDOW_HPP

#include "core/dataset.hpp"
#include "core/blockchain.hpp"

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QVector>

namespace gui {

class GeneratorPage;
class ManualBuilderPage;
class BlockchainPage;
class DataPage;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private Q_SLOTS:
    void onNavButtonClicked(int index);
    void toggleTheme();
    void updateChainStatusChip(int blockCount, bool verified);
    void onDatasetLoaded();

private:
    void setupUi();
    void setupShortcuts();
    void loadSettings();
    void saveSettings();
    void autoLoadSampleData();

    core::Dataset m_dataset;
    core::Blockchain m_blockchain;

    // UI elements
    QWidget* m_sidebar;
    QLabel* m_pageTitleLabel;
    QLabel* m_statusChip;
    QPushButton* m_themeToggleBtn;
    QStackedWidget* m_stackedWidget;
    QVector<QPushButton*> m_navButtons;

    // Pages
    GeneratorPage* m_generatorPage;
    ManualBuilderPage* m_manualPage;
    BlockchainPage* m_blockchainPage;
    DataPage* m_dataPage;
};

} // namespace gui

#endif // TIMETABLE_CHAIN_MAINWINDOW_HPP
