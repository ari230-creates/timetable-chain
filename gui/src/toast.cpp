#include "toast.hpp"
#include "theme.hpp"

#include <QHBoxLayout>
#include <QPainter>
#include <QPropertyAnimation>
#include <QApplication>

namespace gui {

Toast::Toast(QWidget* parent, const QString& message, ToastType type, int durationMs)
    : QWidget(parent), m_message(message), m_type(type) {
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_DeleteOnClose);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(18, 12, 18, 12);

    auto* label = new QLabel(message, this);
    label->setStyleSheet("color: #FFFFFF; font-weight: 600; font-size: 13px;");
    layout->addWidget(label);

    adjustSize();

    // Position at top-center of parent window
    if (parent) {
        int x = (parent->width() - width()) / 2;
        int y = 24;
        move(x, y);
    }

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, [this]() {
        auto* eff = new QGraphicsOpacityEffect(this);
        setGraphicsEffect(eff);
        auto* anim = new QPropertyAnimation(eff, "opacity", this);
        anim->setDuration(300);
        anim->setStartValue(1.0);
        anim->setEndValue(0.0);
        connect(anim, &QPropertyAnimation::finished, this, &QWidget::close);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    });
    m_timer->start(durationMs);

    QWidget::show();
    raise();
}

void Toast::show(QWidget* parent, const QString& message, ToastType type, int durationMs) {
    if (!parent) return;
    new Toast(parent, message, type, durationMs);
}

void Toast::paintEvent(QPaintEvent* /*event*/) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QColor bg;
    switch (m_type) {
        case ToastType::Success: bg = QColor("#2E9E6B"); break;
        case ToastType::Error:   bg = QColor("#D64545"); break;
        case ToastType::Info:    bg = QColor("#5B5BD6"); break;
        case ToastType::Warning: bg = QColor("#E0A030"); break;
    }

    p.setPen(Qt::NoPen);
    p.setBrush(bg);
    p.drawRoundedRect(rect(), 10, 10);
}

} // namespace gui
