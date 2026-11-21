#include "theme.hpp"

#include <QApplication>
#include <QFile>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>

namespace gui {

ThemeMode Theme::s_currentMode = ThemeMode::Light;

void Theme::applyTheme(ThemeMode mode) {
    s_currentMode = mode;
    QString path = (mode == ThemeMode::Light) ? ":/styles/light.qss" : ":/styles/dark.qss";
    QFile file(path);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QString qss = QLatin1String(file.readAll());
        qApp->setStyleSheet(qss);
        file.close();
    }
}

ThemeMode Theme::currentMode() {
    return s_currentMode;
}

bool Theme::isDark() {
    return s_currentMode == ThemeMode::Dark;
}

QColor Theme::backgroundColor() {
    return isDark() ? QColor("#0F1220") : QColor("#F6F7FB");
}

QColor Theme::cardColor() {
    return isDark() ? QColor("#181C2E") : QColor("#FFFFFF");
}

QColor Theme::borderColor() {
    return isDark() ? QColor("#2A2F47") : QColor("#E4E7F0");
}

QColor Theme::textColor() {
    return isDark() ? QColor("#E8EAF6") : QColor("#1B1F3B");
}

QColor Theme::mutedTextColor() {
    return isDark() ? QColor("#9AA0C3") : QColor("#6B7194");
}

QColor Theme::accentColor() {
    return QColor("#5B5BD6");
}

QColor Theme::successColor() {
    return QColor("#2E9E6B");
}

QColor Theme::dangerColor() {
    return QColor("#D64545");
}

QColor Theme::warningColor() {
    return QColor("#E0A030");
}

QColor Theme::subjectColor(int subjectId) {
    // 6 harmonious, distinct, modern card colors
    static const QVector<QColor> lightPalette = {
        QColor("#E8F0FE"), // Soft Blue
        QColor("#F3E8FD"), // Soft Purple
        QColor("#E6F4EA"), // Soft Green
        QColor("#FEF7E0"), // Soft Amber
        QColor("#FCE8E6"), // Soft Coral
        QColor("#E0F2FE")  // Soft Cyan
    };
    static const QVector<QColor> darkPalette = {
        QColor("#1E294B"),
        QColor("#2C204B"),
        QColor("#1B382B"),
        QColor("#38321B"),
        QColor("#3D1E1E"),
        QColor("#1A3245")
    };

    int idx = (subjectId > 0 ? (subjectId - 1) : 0) % 6;
    return isDark() ? darkPalette[idx] : lightPalette[idx];
}

QColor Theme::subjectTextColor(int subjectId) {
    static const QVector<QColor> lightText = {
        QColor("#1A73E8"),
        QColor("#9334E6"),
        QColor("#137333"),
        QColor("#B06000"),
        QColor("#C5221F"),
        QColor("#0284C7")
    };
    static const QVector<QColor> darkText = {
        QColor("#8AB4F8"),
        QColor("#C58AF9"),
        QColor("#81C995"),
        QColor("#FDD663"),
        QColor("#F28B82"),
        QColor("#7DD3FC")
    };
    int idx = (subjectId > 0 ? (subjectId - 1) : 0) % 6;
    return isDark() ? darkText[idx] : lightText[idx];
}

QGraphicsDropShadowEffect* Theme::createCardShadow(QObject* /*parent*/) {
    // Return nullptr to avoid macOS Metal offscreen alpha-blending artifacts
    // (which render black boxes/corners on Retina displays).
    // Elegant crisp borders are applied via QSS instead.
    return nullptr;
}

QIcon Theme::iconGenerator(const QColor& color) {
    QPixmap pixmap(24, 24);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(color, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.setBrush(Qt::NoBrush);

    // Lightning / algorithm spark
    QPolygonF poly;
    poly << QPointF(13, 3) << QPointF(6, 13) << QPointF(12, 13)
         << QPointF(11, 21) << QPointF(18, 11) << QPointF(12, 11);
    p.drawPolygon(poly);
    return QIcon(pixmap);
}

QIcon Theme::iconManual(const QColor& color) {
    QPixmap pixmap(24, 24);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(color, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.setBrush(Qt::NoBrush);

    // Calendar grid with cursor/edit
    p.drawRoundedRect(3, 4, 18, 16, 3, 3);
    p.drawLine(3, 9, 21, 9);
    p.drawLine(8, 2, 8, 6);
    p.drawLine(16, 2, 16, 6);
    p.drawPoint(8, 14);
    p.drawPoint(12, 14);
    p.drawPoint(16, 14);
    return QIcon(pixmap);
}

QIcon Theme::iconBlockchain(const QColor& color) {
    QPixmap pixmap(24, 24);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(color, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.setBrush(Qt::NoBrush);

    // Chain links
    p.drawRoundedRect(4, 4, 8, 8, 2, 2);
    p.drawRoundedRect(12, 12, 8, 8, 2, 2);
    p.drawLine(10, 10, 14, 14);
    return QIcon(pixmap);
}

QIcon Theme::iconData(const QColor& color) {
    QPixmap pixmap(24, 24);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(color, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.setBrush(Qt::NoBrush);

    // Database cylinders
    p.drawEllipse(4, 4, 16, 6);
    p.drawLine(4, 7, 4, 17);
    p.drawLine(20, 7, 20, 17);
    p.drawArc(4, 14, 16, 6, 0, -180 * 16);
    p.drawArc(4, 9, 16, 6, 0, -180 * 16);
    return QIcon(pixmap);
}

QIcon Theme::iconTheme(bool dark) {
    QPixmap pixmap(24, 24);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);

    if (dark) {
        // Sun icon (click to switch to light)
        p.setPen(QPen(QColor("#E8EAF6"), 2, Qt::SolidLine, Qt::RoundCap));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(7, 7, 10, 10);
        // rays
        p.drawLine(12, 2, 12, 4);
        p.drawLine(12, 20, 12, 22);
        p.drawLine(2, 12, 4, 12);
        p.drawLine(20, 12, 22, 12);
        p.drawLine(5, 5, 6, 6);
        p.drawLine(18, 18, 19, 19);
        p.drawLine(5, 19, 6, 18);
        p.drawLine(18, 5, 19, 6);
    } else {
        // Moon icon (click to switch to dark)
        p.setPen(QPen(QColor("#1B1F3B"), 2, Qt::SolidLine, Qt::RoundCap));
        p.setBrush(Qt::NoBrush);
        QPainterPath path;
        path.moveTo(18, 14);
        path.arcTo(6, 4, 14, 16, -30, 240);
        path.arcTo(9, 7, 10, 10, 120, -150);
        p.drawPath(path);
    }
    return QIcon(pixmap);
}

QIcon Theme::iconCheck(const QColor& color) {
    QPixmap pixmap(20, 20);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(color, 2.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawLine(4, 10, 8, 15);
    p.drawLine(8, 15, 16, 5);
    return QIcon(pixmap);
}

QIcon Theme::iconAlert(const QColor& color) {
    QPixmap pixmap(20, 20);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(color, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    QPolygonF tri;
    tri << QPointF(10, 3) << QPointF(2, 17) << QPointF(18, 17);
    p.drawPolygon(tri);
    p.drawLine(10, 8, 10, 12);
    p.drawPoint(10, 15);
    return QIcon(pixmap);
}

} // namespace gui
