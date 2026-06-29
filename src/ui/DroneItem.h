#ifndef DRONEITEM_H
#define DRONEITEM_H

#include <QGraphicsObject>

class Drone;

class DroneItem : public QGraphicsObject {
    Q_OBJECT
public:
    explicit DroneItem(Drone *drone, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    Drone *drone() const { return m_drone; }
    void syncPosition();

private:
    Drone *m_drone;
    static constexpr double RADIUS = 16.0;
};

#endif // DRONEITEM_H
