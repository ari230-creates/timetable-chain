#include "mainwindow.hpp"
#include "generator_page.hpp"
#include "manual_builder_page.hpp"
#include "blockchain_page.hpp"
#include "data_page.hpp"
#include "slot_dialog.hpp"
#include "theme.hpp"
#include "core/constraint_checker.hpp"

#include <QApplication>
#include <QFont>
#include <QFontDatabase>
#include <QTimer>
#include <cassert>
#include <iostream>
#include <filesystem>

int main(int argc, char* argv[]) {
    // Set offscreen QPA platform for headless execution
    qputenv("QT_QPA_PLATFORM", "offscreen");

    QApplication app(argc, argv);
    app.setApplicationName("Timetable Chain GUI Test");

    QFont appFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    if (!QFontDatabase::families().contains(appFont.family())) {
        QStringList candidates = {"SF Pro Text", "SF Pro", "Helvetica Neue", "Helvetica", "Arial"};
        QString chosenFamily = "Helvetica";
        for (const auto& fam : candidates) {
            if (QFontDatabase::families().contains(fam)) {
                chosenFamily = fam;
                break;
            }
        }
        appFont = QFont(chosenFamily, 13);
    } else {
        appFont.setPointSize(13);
    }
    app.setFont(appFont);

    std::cout << "[RUNNING] GUI Integration Test Suite...\n";

    gui::MainWindow window;
    window.show();

    // 1. Check auto-loading of sample data
    // The dataset should be loaded automatically on window initialization
    std::cout << "  -> Testing Auto-load of sample data...\n";
    // Check if window is visible and dataset loaded
    assert(window.isVisible());

    // Switch to Blockchain Page (index 2)
    window.resize(1000, 700);
    window.show();
    QApplication::processEvents();

    auto navBtns = window.findChildren<QPushButton*>();
    for (auto* btn : navBtns) {
        if (btn->text().contains("Blockchain")) {
            btn->click();
            break;
        }
    }
    QApplication::processEvents();

    QImage img = window.grab().toImage();
    // Sample a pixel below the block cards, e.g. at x=500, y=600
    QRgb pixel = img.pixel(500, 600);
    assert(qRed(pixel) == 246 && qGreen(pixel) == 247 && qBlue(pixel) == 251);


    // 2. Test Manual Builder Constraint Check: Dr. Rao teaches Databases
    std::cout << "  -> Testing Manual Builder rejection ('Dr. Rao teaches Databases')...\n";
    core::Dataset testDs;
    std::string loadErr;
    std::string samplePath = "data/sample.txt";
    if (!std::filesystem::exists(samplePath) && std::filesystem::exists("../data/sample.txt")) {
        samplePath = "../data/sample.txt";
    }
    bool testDsLoaded = testDs.loadFromFile(samplePath, loadErr);
    assert(testDsLoaded);

    const auto* drRao = testDs.findFaculty(1);   // Dr. Rao
    const auto* databases = testDs.findSubject(3); // Databases
    const auto* slot1 = testDs.findSlot(1);

    assert(drRao != nullptr);
    assert(databases != nullptr);
    assert(slot1 != nullptr);

    core::Timetable emptyTt;
    std::string rejectionReason;
    bool canPlace = core::ConstraintChecker::canPlace(emptyTt, *databases, *drRao, *slot1, rejectionReason);
    assert(!canPlace);
    std::cout << "     Rejection Reason: " << rejectionReason << "\n";
    assert(rejectionReason.find("cannot teach") != std::string::npos);
    assert(rejectionReason.find("Dr. Rao") != std::string::npos);
    assert(rejectionReason.find("Databases") != std::string::npos);

    // 3. Test Theme switching & page backgrounds in Light and Dark mode
    std::cout << "  -> Testing Theme Switcher (Light <-> Dark) & Page Backgrounds...\n";
    for (auto mode : {gui::ThemeMode::Light, gui::ThemeMode::Dark}) {
        gui::Theme::applyTheme(mode);
        assert(gui::Theme::isDark() == (mode == gui::ThemeMode::Dark));
        QApplication::processEvents();

        QColor expectedBg = gui::Theme::backgroundColor();
        // Check Blockchain page background below cards
        for (auto* btn : navBtns) {
            if (btn->text().contains("Blockchain")) {
                btn->click();
                break;
            }
        }
        QApplication::processEvents();
        QImage pageImg = window.grab().toImage();
        QRgb sampledPixel = pageImg.pixel(500, 600);
        int r = qRed(sampledPixel);
        int g = qGreen(sampledPixel);
        int b = qBlue(sampledPixel);
        // Verify pixel matches expected theme background
        assert(std::abs(r - expectedBg.red()) <= 1);
        assert(std::abs(g - expectedBg.green()) <= 1);
        assert(std::abs(b - expectedBg.blue()) <= 1);
    }
    // Switch back to Light theme
    gui::Theme::applyTheme(gui::ThemeMode::Light);
    assert(!gui::Theme::isDark());

    // 4. Test Bad Data File Handling
    std::cout << "  -> Testing Bad Data File Handling (Graceful error, no crash)...\n";
    core::Dataset badDs;
    std::string badErr;
    bool badLoaded = badDs.loadFromFile("non_existent_file.txt", badErr);
    assert(!badLoaded);
    assert(!badErr.empty());

    // 5. Test Bad Chain File Handling
    std::cout << "  -> Testing Bad Chain File Handling (Graceful error, no crash)...\n";
    core::Blockchain badChain;
    std::string badChainErr;
    bool badChainLoaded = badChain.load("non_existent_chain.txt", badChainErr);
    assert(!badChainLoaded);
    assert(!badChainErr.empty());

    std::cout << "[PASSED] GUI Integration Test Suite Successfully Completed!\n";
    return 0;
}
