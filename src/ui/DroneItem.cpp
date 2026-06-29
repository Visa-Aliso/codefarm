#include "DroneItem.h"
#include "core/Drone.h"
#include <QPainter>
#include <QRadialGradient>

DroneItem::DroneItem(Drone *drone, QGraphicsItem *parent)
    : QGraphicsObject(parent)
    , m_drone(drone)
{
}

QRectF DroneItem::boundingRect() const {
    return QRectF(-RADIUS - 4, -RADIUS - 4, 2 * RADIUS + 8, 2 * RADIUS + 8);
}

void DroneItem::paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *,
                       QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing);

    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(70, 88, 74, 35));
    painter->drawEllipse(QPointF(0, 14), RADIUS * 1.15, RADIUS * 0.35);

    painter->setBrush(QColor("#D7EAF0"));
    painter->setPen(QPen(QColor("#7FA9B9"), 1.2));
    painter->drawRoundedRect(QRectF(-24, -5, 13, 9), 5, 5);
    painter->drawRoundedRect(QRectF(11, -5, 13, 9), 5, 5);

    painter->setPen(QPen(QColor("#9ABFCA"), 2.0, Qt::SolidLine, Qt::RoundCap));
    painter->drawLine(QPointF(-28, -9), QPointF(-8, -9));
    painter->drawLine(QPointF(8, -9), QPointF(28, -9));
    painter->setPen(QPen(QColor("#CFE8EE"), 4.0, Qt::SolidLine, Qt::RoundCap));
    painter->drawLine(QPointF(-30, -12), QPointF(-10, -6));
    painter->drawLine(QPointF(10, -6), QPointF(30, -12));

    QRadialGradient body(QPointF(-3, -4), RADIUS);
    body.setColorAt(0.0, QColor("#F8FCFF"));
    body.setColorAt(0.62, QColor("#C9E7F1"));
    body.setColorAt(1.0, QColor("#82B9CC"));
    painter->setBrush(body);
    painter->setPen(QPen(QColor("#6E9FB2"), 1.6));
    painter->drawRoundedRect(QRectF(-17, -16, 34, 30), 15, 15);

    painter->setBrush(QColor("#EBF8FB"));
    painter->setPen(QPen(QColor("#79A8B7"), 1));
    painter->drawRoundedRect(QRectF(-12, -9, 24, 11), 5, 5);

    painter->setBrush(QColor("#315A6A"));
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(QPointF(-6, -4), 2.5, 2.5);
    painter->drawEllipse(QPointF(6, -4), 2.5, 2.5);

    QColor stateColor = QColor("#A8D8EA"); // idle
    switch (m_drone->state()) {
    case DroneState::Moving: stateColor = QColor("#8FBC8F"); break;
    case DroneState::Working: stateColor = QColor("#E8C86A"); break;
    default: break;
    }
    painter->setBrush(stateColor);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(QRectF(-6, 7, 12, 5), 2, 2);
}

void DroneItem::syncPosition() {
    // Position is set externally by MapView or animation
}
