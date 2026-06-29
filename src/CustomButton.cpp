#include "CustomButton.h"

CustomButton::CustomButton(const QString &text, QWidget *parent) : QPushButton(text, parent)
{
    setCursor(Qt::PointingHandCursor);
    setMinimumHeight(42);

    shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(10);
    shadow->setOffset(0, 2);
    shadow->setColor(QColor(131, 155, 99, 35));
    setGraphicsEffect(shadow);
}

void CustomButton::enterEvent(QEnterEvent *event)
{
    QPushButton::enterEvent(event);
    animateShadow(16, 4, QColor(153, 184, 101, 55));
}

void CustomButton::leaveEvent(QEvent *event)
{
    QPushButton::leaveEvent(event);
    animateShadow(10, 2, QColor(131, 155, 99, 35));
}

void CustomButton::mousePressEvent(QMouseEvent *event)
{
    QPushButton::mousePressEvent(event);
    animateShadow(5, 1, QColor(123, 140, 92, 28));
}

void CustomButton::mouseReleaseEvent(QMouseEvent *event)
{
    QPushButton::mouseReleaseEvent(event);
    animateShadow(16, 4, QColor(153, 184, 101, 55));
}

void CustomButton::animateShadow(qreal blur, qreal y, const QColor &color)
{
    auto *blurAnim = new QPropertyAnimation(shadow, "blurRadius", shadow);
    blurAnim->setDuration(150);
    blurAnim->setStartValue(shadow->blurRadius());
    blurAnim->setEndValue(blur);
    blurAnim->setEasingCurve(QEasingCurve::InOutCubic);
    blurAnim->start(QAbstractAnimation::DeleteWhenStopped);

    auto *offsetAnim = new QPropertyAnimation(shadow, "offset", shadow);
    offsetAnim->setDuration(150);
    offsetAnim->setStartValue(shadow->offset());
    offsetAnim->setEndValue(QPointF(0, y));
    offsetAnim->setEasingCurve(QEasingCurve::InOutCubic);
    offsetAnim->start(QAbstractAnimation::DeleteWhenStopped);

    shadow->setColor(color);
}
