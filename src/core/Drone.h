#ifndef DRONE_H
#define DRONE_H

#include <QObject>
#include <QPointF>
#include <QQueue>
#include <QString>

enum class DroneTask {
    Move, Plant, Water, Fertilize, Harvest, DebugPlot, Cultivate, Wait
};

inline QString droneTaskName(DroneTask t) {
    switch (t) {
    case DroneTask::Move:       return QStringLiteral("移动");
    case DroneTask::Plant:      return QStringLiteral("种植");
    case DroneTask::Water:      return QStringLiteral("浇水");
    case DroneTask::Fertilize:  return QStringLiteral("施肥");
    case DroneTask::Harvest:    return QStringLiteral("收割");
    case DroneTask::DebugPlot:  return QStringLiteral("除虫");
    case DroneTask::Cultivate:  return QStringLiteral("开垦");
    case DroneTask::Wait:       return QStringLiteral("等待");
    default: return QStringLiteral("空闲");
    }
}

enum class DroneState { Idle, Moving, Working };

struct DroneTaskItem {
    DroneTask task;
    int targetX;
    int targetY;
    int waitMs = 0;
    int cropType = 0;
};

class Drone : public QObject {
    Q_OBJECT
public:
    explicit Drone(int id, QObject *parent = nullptr);

    int id() const { return m_id; }
    QPointF position() const { return m_pos; }
    void setPosition(QPointF pos) { m_pos = pos; }

    double speed() const { return m_baseSpeed * m_speedMultiplier; }
    void setBaseSpeed(double s) { m_baseSpeed = s; }
    void setSpeedMultiplier(double m) { m_speedMultiplier = m; }

    double actionSpeed() const { return m_actionSpeed; }
    void setActionSpeed(double s) { m_actionSpeed = s; }

    DroneState state() const { return m_state; }
    void setState(DroneState s) { m_state = s; }

    double workTimer() const { return m_workTimer; }
    void addWorkTime(double dt) { m_workTimer += dt; }
    void resetWorkTimer() { m_workTimer = 0.0; }

    void enqueueTask(DroneTask task, int x, int y);
    void enqueuePlant(int x, int y, int cropType);
    void enqueueWait(int ms);
    DroneTaskItem dequeueTask();
    bool hasTasks() const { return !m_taskQueue.isEmpty(); }
    DroneTaskItem currentTask() const;
    void clearTasks() { m_taskQueue.clear(); }

    int taskCount() const { return m_taskQueue.size(); }
    QPointF plannedPosition() const;

    // Serialization
    QJsonObject toJson() const;
    static Drone *fromJson(const QJsonObject &obj, QObject *parent = nullptr);

signals:
    void stateChanged(int droneId);
    void positionChanged(int droneId);

private:
    int m_id;
    QPointF m_pos;
    double m_baseSpeed = 1.0;
    double m_speedMultiplier = 1.0;
    double m_actionSpeed = 1.0;
    DroneState m_state = DroneState::Idle;
    double m_workTimer = 0.0;
    QQueue<DroneTaskItem> m_taskQueue;
};

#endif // DRONE_H
