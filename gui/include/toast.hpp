#ifndef TIMETABLE_CHAIN_TOAST_HPP
#define TIMETABLE_CHAIN_TOAST_HPP

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QGraphicsOpacityEffect>

namespace gui {

enum class ToastType {
    Success,
    Error,
    Info,
    Warning
};

class Toast : public QWidget {
    Q_OBJECT

public:
    static void show(QWidget* parent, const QString& message, ToastType type = ToastType::Info, int durationMs = 3200);

private:
    Toast(QWidget* parent, const QString& message, ToastType type, int durationMs);
    void paintEvent(QPaintEvent* event) override;

    QString m_message;
    ToastType m_type;
    QTimer* m_timer;
};

} // namespace gui

#endif // TIMETABLE_CHAIN_TOAST_HPP
