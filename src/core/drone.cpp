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
    if (!consumeEnergy(1.0f)) return false;

    x_ = nx; y_ = ny;
    emit positionChanged(x_, y_);
    return true;
}

bool Drone::consumeEnergy(float amount) {
    if (energy_ < amount) return false;
    energy_ -= amount;
    emit energyChanged();
    return true;
}

void Drone::regenEnergy() {
    energy_ = qMin(energy_ + energyRegen_, maxEnergy_);
    emit energyChanged();
}

void Drone::reset(int startX, int startY, float maxE) {
    x_ = startX; y_ = startY;
    maxEnergy_ = maxE;
    energy_ = maxE;
    emit positionChanged(x_, y_);
    emit energyChanged();
}
