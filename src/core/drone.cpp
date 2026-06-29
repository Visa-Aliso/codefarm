#include "drone.h"

Drone::Drone(QObject *parent) : QObject(parent) {}

bool Drone::move(const QString &direction, int mapW, int mapH) {
    int nx = x_, ny = y_;
    if (direction == "up") ny--;
    else if (direction == "down") ny++;
    else if (direction == "left") nx--;
    else if (direction == "right") nx++;
    else return false;

    if (nx < 0 || nx >= mapW || ny < 0 || ny >= mapH) return false;

    x_ = nx; y_ = ny;
    emit positionChanged(x_, y_);
    return true;
}

void Drone::reset(int startX, int startY) {
    x_ = startX; y_ = startY;
    emit positionChanged(x_, y_);
}
