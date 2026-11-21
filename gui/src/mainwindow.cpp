#include "mainwindow.hpp"
#include "generator_page.hpp"
#include "manual_builder_page.hpp"
#include "blockchain_page.hpp"
#include "data_page.hpp"
#include "theme.hpp"
#include "toast.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QShortcut>
#include <QKeySequence>
#include <QSettings>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
#include <QStatusBar>

namespace gui {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setMinimumSize(1020, 680);
    setWindowTitle("Timetable Chain - Intelligent Timetable Generator & Ledger");

    setupUi();
    setupShortcuts();
    loadSettings();
    autoLoadSampleData();
    updateChainStatusChip(static_cast<int>(m_blockchain.size()), false);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi() {
    auto* centralWidget = new QWidget(this);
    centralWidget->setObjectName("CentralWidget");
    centralWidget->setAttribute(Qt::WA_StyledBackground, true);
    centralWidget->setAutoFillBackground(true);
    setCentralWidget(centralWidget);

    auto* rootLayout = new QHBoxLayout(centralWidget);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // ==========================================
    // Left Sidebar (Fixed ~200px)
    // ==========================================
    m_sidebar = new QWidget(centralWidget);
    m_sidebar->setObjectName("Sidebar");
    m_sidebar->setAttribute(Qt::WA_StyledBackground, true);
    m_sidebar->setAutoFillBackground(true);
    m_sidebar->setFixedWidth(210);

    auto* sidebarLayout = new QVBoxLayout(m_sidebar);
    sidebarLayout->setContentsMargins(14, 20, 14, 20);
    sidebarLayout->setSpacing(6);

    // App Branding Header
    auto* brandContainer = new QWidget(m_sidebar);
    brandContainer->setAttribute(Qt::WA_StyledBackground, true);
    auto* brandLayout = new QVBoxLayout(brandContainer);
    brandLayout->setContentsMargins(8, 0, 8, 16);
    brandLayout->setSpacing(2);

    auto* appTitle = new QLabel("TIMETABLE CHAIN", brandContainer);
    appTitle->setStyleSheet("font-size: 14px; font-weight: 800; color: #5B5BD6; letter-spacing: 0.5px;");
    brandLayout->addWidget(appTitle);

    auto* appSubtitle = new QLabel("C++ Project & Ledger", brandContainer);
    appSubtitle->setStyleSheet("font-size: 11px; font-weight: 500; color: #6B7194;");
    brandLayout->addWidget(appSubtitle);

    sidebarLayout->addWidget(brandContainer);

    // Navigation buttons
    auto createNavBtn = [this, sidebarLayout](const QString& text, const QIcon& icon, int index) {
        auto* btn = new QPushButton(text, m_sidebar);
        btn->setProperty("class", "nav-btn");
        btn->setIcon(icon);
        btn->setIconSize(QSize(18, 18));
        btn->setCursor(Qt::PointingHandCursor);
        connect(btn, &QPushButton::clicked, this, [this, index]() {
            onNavButtonClicked(index);
        });
        sidebarLayout->addWidget(btn);
        m_navButtons.push_back(btn);
    };

    createNavBtn(" Generator", Theme::iconGenerator(Theme::accentColor()), 0);
    createNavBtn(" Manual Builder", Theme::iconManual(Theme::accentColor()), 1);
    createNavBtn(" Blockchain", Theme::iconBlockchain(Theme::accentColor()), 2);
    createNavBtn(" Curriculum Data", Theme::iconData(Theme::accentColor()), 3);

    sidebarLayout->addStretch();
    rootLayout->addWidget(m_sidebar);

    // ==========================================
    // Right Main Area (TopBar + QStackedWidget)
    // ==========================================
    auto* rightArea = new QWidget(centralWidget);
    rightArea->setObjectName("BackgroundWidget");
    rightArea->setAttribute(Qt::WA_StyledBackground, true);
    rightArea->setAutoFillBackground(true);
    auto* rightLayout = new QVBoxLayout(rightArea);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    // Top Bar
    auto* topBar = new QWidget(rightArea);
    topBar->setObjectName("TopBar");
    topBar->setAttribute(Qt::WA_StyledBackground, true);
    topBar->setAutoFillBackground(true);
    topBar->setFixedHeight(60);
    auto* topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(24, 0, 24, 0);
    topBarLayout->setSpacing(16);

    m_pageTitleLabel = new QLabel("Timetable Generator", topBar);
    m_pageTitleLabel->setObjectName("PageTitle");
    topBarLayout->addWidget(m_pageTitleLabel);

    topBarLayout->addStretch();

    m_statusChip = new QLabel("Chain: 1 block • Unverified", topBar);
    m_statusChip->setObjectName("StatusChip");
    topBarLayout->addWidget(m_statusChip);

    m_themeToggleBtn = new QPushButton(topBar);
    m_themeToggleBtn->setIcon(Theme::iconTheme(Theme::isDark()));
    m_themeToggleBtn->setIconSize(QSize(18, 18));
    m_themeToggleBtn->setToolTip("Toggle Light / Dark Theme");
    m_themeToggleBtn->setCursor(Qt::PointingHandCursor);
    m_themeToggleBtn->setFixedSize(36, 36);
    connect(m_themeToggleBtn, &QPushButton::clicked, this, &MainWindow::toggleTheme);
    topBarLayout->addWidget(m_themeToggleBtn);

    rightLayout->addWidget(topBar);

    // Central QStackedWidget
    m_stackedWidget = new QStackedWidget(rightArea);
    m_stackedWidget->setObjectName("StackedWidget");
    m_stackedWidget->setAttribute(Qt::WA_StyledBackground, true);
    m_stackedWidget->setAutoFillBackground(true);

    m_generatorPage = new GeneratorPage(m_dataset, m_blockchain, this);
    m_manualPage = new ManualBuilderPage(m_dataset, m_blockchain, this);
    m_blockchainPage = new BlockchainPage(m_blockchain, m_dataset, this);
    m_dataPage = new DataPage(m_dataset, this);

    m_stackedWidget->addWidget(m_generatorPage);
    m_stackedWidget->addWidget(m_manualPage);
    m_stackedWidget->addWidget(m_blockchainPage);
    m_stackedWidget->addWidget(m_dataPage);

    rightLayout->addWidget(m_stackedWidget, 1);
    rootLayout->addWidget(rightArea, 1);

    // Cross-page signals
    connect(m_dataPage, &DataPage::datasetLoaded, this, &MainWindow::onDatasetLoaded);
    connect(m_generatorPage, &GeneratorPage::blockAdded, m_blockchainPage, &BlockchainPage::refreshBlocks);
    connect(m_manualPage, &ManualBuilderPage::blockAdded, m_blockchainPage, &BlockchainPage::refreshBlocks);
    connect(m_generatorPage, &GeneratorPage::blockAdded, this, [this]() {
        updateChainStatusChip(static_cast<int>(m_blockchain.size()), false);
    });
    connect(m_manualPage, &ManualBuilderPage::blockAdded, this, [this]() {
        updateChainStatusChip(static_cast<int>(m_blockchain.size()), false);
    });
    connect(m_blockchainPage, &BlockchainPage::chainStatusUpdated, this, &MainWindow::updateChainStatusChip);

    // Status Bar
    statusBar()->showMessage("Ready");

    // Select Generator page by default
    onNavButtonClicked(0);
}

void MainWindow::setupShortcuts() {
    // Cmd/Ctrl+G -> Generate
    auto* genShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_G), this);
    connect(genShortcut, &QShortcut::activated, this, [this]() {
        onNavButtonClicked(0);
        m_generatorPage->startGeneration();
    });

    // Cmd/Ctrl+L -> Load data
    auto* loadShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_L), this);
    connect(loadShortcut, &QShortcut::activated, this, [this]() {
        onNavButtonClicked(3);
        m_dataPage->openLoadFileDialog();
    });

    // Cmd/Ctrl+Shift+V -> Verify blockchain
    auto* verifyShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_V), this);
    connect(verifyShortcut, &QShortcut::activated, this, [this]() {
        onNavButtonClicked(2);
        m_blockchainPage->verifyChain();
    });
}

void MainWindow::onNavButtonClicked(int index) {
    if (index < 0 || index >= m_stackedWidget->count()) return;
    m_stackedWidget->setCurrentIndex(index);

    // Update active style on sidebar buttons
    for (int i = 0; i < m_navButtons.size(); ++i) {
        bool active = (i == index);
        m_navButtons[i]->setProperty("active", active ? "true" : "false");
        m_navButtons[i]->style()->unpolish(m_navButtons[i]);
        m_navButtons[i]->style()->polish(m_navButtons[i]);
    }

    // Update top bar title
    switch (index) {
        case 0: m_pageTitleLabel->setText("Timetable Generator"); break;
        case 1: m_pageTitleLabel->setText("Manual Schedule Builder"); break;
        case 2: m_pageTitleLabel->setText("Blockchain Verification Ledger"); break;
        case 3: m_pageTitleLabel->setText("Curriculum Dataset"); break;
    }
}

void MainWindow::toggleTheme() {
    ThemeMode next = Theme::isDark() ? ThemeMode::Light : ThemeMode::Dark;
    Theme::applyTheme(next);
    m_themeToggleBtn->setIcon(Theme::iconTheme(Theme::isDark()));
    statusBar()->showMessage(Theme::isDark() ? "Dark theme applied" : "Light theme applied", 2000);
}

void MainWindow::updateChainStatusChip(int blockCount, bool verified) {
    if (verified) {
        m_statusChip->setText(QString("Chain: %1 block%2 • Verified ✓")
                                  .arg(blockCount)
                                  .arg(blockCount == 1 ? "" : "s"));
        m_statusChip->setStyleSheet("background-color: #E6F4EA; color: #137333; border: 1px solid #CEEAD6; border-radius: 12px; padding: 4px 12px; font-weight: 600; font-size: 12px;");
    } else {
        m_statusChip->setText(QString("Chain: %1 block%2 • Unverified")
                                  .arg(blockCount)
                                  .arg(blockCount == 1 ? "" : "s"));
        m_statusChip->setStyleSheet("background-color: #EEF0FD; color: #5B5BD6; border: 1px solid #D6D8F8; border-radius: 12px; padding: 4px 12px; font-weight: 600; font-size: 12px;");
    }
}

void MainWindow::onDatasetLoaded() {
    m_generatorPage->onDatasetLoaded();
    m_manualPage->onDatasetLoaded();
    statusBar()->showMessage("Curriculum dataset loaded successfully", 3000);
}

void MainWindow::autoLoadSampleData() {
    // Check multiple candidate locations for data/sample.txt
    QString appDir = QCoreApplication::applicationDirPath();
    QStringList candidates = {
        "data/sample.txt",
        "../data/sample.txt",
        "../../data/sample.txt",
        appDir + "/data/sample.txt",
        appDir + "/../data/sample.txt",
        appDir + "/../../data/sample.txt"
    };

    for (const auto& path : candidates) {
        if (QFileInfo::exists(path)) {
            m_dataPage->loadFile(path);
            break;
        }
    }
}

void MainWindow::loadSettings() {
    QSettings settings("TimetableChainProject", "TimetableChain");
    restoreGeometry(settings.value("geometry").toByteArray());
    bool isDark = settings.value("darkMode", false).toBool();
    Theme::applyTheme(isDark ? ThemeMode::Dark : ThemeMode::Light);
    m_themeToggleBtn->setIcon(Theme::iconTheme(Theme::isDark()));
}

void MainWindow::saveSettings() {
    QSettings settings("TimetableChainProject", "TimetableChain");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("darkMode", Theme::isDark());
}

void MainWindow::closeEvent(QCloseEvent* event) {
    saveSettings();
    QMainWindow::closeEvent(event);
}

} // namespace gui
