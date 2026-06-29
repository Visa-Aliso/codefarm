#include "Drone.h"
#include <QJsonObject>
#include <QJsonArray>

Drone::Drone(int id, QObject *parent)
    : QObject(parent), m_id(id)
{
}

void Drone::enqueueTask(DroneTask task, int x, int y) {
    m_taskQueue.enqueue({task, x, y, 0, 0});
}

void Drone::enqueuePlant(int x, int y, int cropType) {
    m_taskQueue.enqueue({DroneTask::Plant, x, y, 0, cropType});
}

void Drone::enqueueWait(int ms) {
    const QPointF p = plannedPosition();
    m_taskQueue.enqueue({DroneTask::Wait, static_cast<int>(p.x()), static_cast<int>(p.y()), ms, 0});
}

DroneTaskItem Drone::dequeueTask() {
    return m_taskQueue.dequeue();
}

DroneTaskItem Drone::currentTask() const {
    return m_taskQueue.isEmpty() ? DroneTaskItem{DroneTask::Move, -1, -1, 0, 0} : m_taskQueue.head();
}

QPointF Drone::plannedPosition() const {
    QPointF p = m_pos;
    for (const auto &task : m_taskQueue) {
        if (task.targetX >= 0 && task.targetY >= 0)
            p = QPointF(task.targetX, task.targetY);
    }
    return p;
}

QJsonObject Drone::toJson() const {
    QJsonObject obj;
    obj["id"] = m_id;
    obj["x"] = m_pos.x();
    obj["y"] = m_pos.y();
    obj["baseSpeed"] = m_baseSpeed;
    obj["speedMultiplier"] = m_speedMultiplier;
    obj["actionSpeed"] = m_actionSpeed;
    return obj;
}

Drone *Drone::fromJson(const QJsonObject &obj, QObject *parent) {
    auto *d = new Drone(obj["id"].toInt(), parent);
    d->setPosition(QPointF(obj["x"].toDouble(), obj["y"].toDouble()));
    d->setBaseSpeed(obj["baseSpeed"].toDouble(1.0));
    d->setSpeedMultiplier(obj["speedMultiplier"].toDouble(1.0));
    d->setActionSpeed(obj["actionSpeed"].toDouble(1.0));
    return d;
}
