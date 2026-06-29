#ifndef DRONE_H
#define DRONE_H

#include <QObject>

class Drone : public QObject {
    Q_OBJECT
    Q_PROPERTY(int x READ x NOTIFY positionChanged)
    Q_PROPERTY(int y READ y NOTIFY positionChanged)

public:
    explicit Drone(QObject *parent = nullptr);

    int x() const { return x_; }
    int y() const { return y_; }

    bool move(const QString &direction, int mapW, int mapH);
    void reset(int startX, int startY);

signals:
    void positionChanged(int x, int y);

private:
    int x_ = 0;
    int y_ = 0;
};

#endif // DRONE_H
