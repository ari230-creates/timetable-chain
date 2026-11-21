#ifndef TIMETABLE_CHAIN_THEME_HPP
#define TIMETABLE_CHAIN_THEME_HPP

#include <QColor>
#include <QIcon>
#include <QGraphicsDropShadowEffect>

namespace gui {

enum class ThemeMode {
    Light,
    Dark
};

class Theme {
public:
    static void applyTheme(ThemeMode mode);
    static ThemeMode currentMode();
    static bool isDark();

    // Palette getters
    static QColor backgroundColor();
    static QColor cardColor();
    static QColor borderColor();
    static QColor textColor();
    static QColor mutedTextColor();
    static QColor accentColor();
    static QColor successColor();
    static QColor dangerColor();
    static QColor warningColor();

    // Stable distinctive palette for subjects
    static QColor subjectColor(int subjectId);
    static QColor subjectTextColor(int subjectId);

    // Helpers
    static QGraphicsDropShadowEffect* createCardShadow(QObject* parent = nullptr);

    // Icon generator using QPainter (ensuring no external image files are needed)
    static QIcon iconGenerator(const QColor& color);
    static QIcon iconManual(const QColor& color);
    static QIcon iconBlockchain(const QColor& color);
    static QIcon iconData(const QColor& color);
    static QIcon iconTheme(bool dark);
    static QIcon iconCheck(const QColor& color);
    static QIcon iconAlert(const QColor& color);

private:
    static ThemeMode s_currentMode;
};

} // namespace gui

#endif // TIMETABLE_CHAIN_THEME_HPP
