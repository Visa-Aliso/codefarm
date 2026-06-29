#ifndef DRONE_H
#define DRONE_H

#include <QObject>

class Drone : public QObject {
    Q_OBJECT
    Q_PROPERTY(int x READ x NOTIFY positionChanged)
    Q_PROPERTY(int y READ y NOTIFY positionChanged)
    Q_PROPERTY(float energy READ energy NOTIFY energyChanged)
    Q_PROPERTY(float maxEnergy READ maxEnergy CONSTANT)

public:
    explicit Drone(QObject *parent = nullptr);

    int x() const { return x_; }
    int y() const { return y_; }
    float energy() const { return energy_; }
    float maxEnergy() const { return maxEnergy_; }

    bool move(const QString &direction, int mapW, int mapH);
    bool consumeEnergy(float amount);
    void regenEnergy();
    void reset(int startX, int startY, float maxE);

signals:
    void positionChanged(int x, int y);
    void energyChanged();

private:
    int x_ = 0;
    int y_ = 0;
    float energy_ = 20.0f;
    float maxEnergy_ = 20.0f;
    float energyRegen_ = 0.5f;
};

#endif // DRONE_H
