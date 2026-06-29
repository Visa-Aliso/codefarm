#pragma once

#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QEnterEvent>

class CustomButton final : public QPushButton {
    Q_OBJECT
public:
    explicit CustomButton(const QString &text, QWidget *parent = nullptr);

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QGraphicsDropShadowEffect *shadow = nullptr;
    void animateShadow(qreal blur, qreal y, const QColor &color);
};
