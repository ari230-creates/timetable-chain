#include "mainwindow.hpp"
#include <QApplication>
#include <QFont>
#include <QFontDatabase>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Timetable Chain");
    app.setOrganizationName("TimetableChainProject");

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

    gui::MainWindow window;
    window.show();

    return app.exec();
}
