#include "ScriptAPI.h"
#include "core/GameState.h"
#include "core/Drone.h"
#include "core/PathFinder.h"
#include "core/CropData.h"
#include <QJsonArray>

ScriptAPI::ScriptAPI(GameState *state, QObject *parent)
    : QObject(parent)
    , m_gameState(state)
{
}

QJsonObject ScriptAPI::init()
{
    m_gameState->resetFarmRuntime();
    QJsonObject result;
    result["ok"] = true;
    result["message"] = "田地和无人机已恢复到当前等级的初始运行状态";
    return result;
}

QJsonObject ScriptAPI::plant(int x, int y) {
    QJsonObject result;
    if (x < 0 || x >= m_gameState->mapWidth() || y < 0 || y >= m_gameState->mapHeight()) {
        result["ok"] = false;
        result["error"] = "坐标超出范围";
        return result;
    }
    LandPlot &plot = m_gameState->plot(x, y);
    if (plot.state != LandState::Empty && plot.state != LandState::Uncultivated) {
        result["ok"] = false;
        result["error"] = "土地无法种植";
        return result;
    }
    // Assign first drone to plant
    if (!m_gameState->drones().isEmpty()) {
        m_gameState->drones().first()->enqueuePlant(x, y, static_cast<int>(CropType::Apple));
    }
    result["ok"] = true;
    return result;
}

int ScriptAPI::cropTypeFromName(const QString &name) const
{
    const QString key = name.trimmed().toLower();
    if (key == "strawberry" || key == "草莓") return static_cast<int>(CropType::Apple);
    if (key == "tomato" || key == "番茄") return static_cast<int>(CropType::Grape);
    if (key == "grape" || key == "葡萄") return static_cast<int>(CropType::Banana);
    if (key == "watermelon" || key == "西瓜") return static_cast<int>(CropType::Watermelon);
    if (key == "pineapple" || key == "菠萝") return static_cast<int>(CropType::Pineapple);
    return static_cast<int>(CropType::Apple);
}

QJsonObject ScriptAPI::plantCurrent(const QString &cropName)
{
    QJsonObject result;
    if (m_gameState->drones().isEmpty()) { result["ok"] = false; result["error"] = "没有可用无人机"; return result; }
    Drone *d = m_gameState->drones().first();
    const QPointF p = d->plannedPosition();
    const int x = static_cast<int>(p.x());
    const int y = static_cast<int>(p.y());
    if (x < 0 || x >= m_gameState->mapWidth() || y < 0 || y >= m_gameState->mapHeight()) {
        result["ok"] = false; result["error"] = "无人机当前位置超出地图"; return result;
    }
    const LandPlot &plot = m_gameState->plot(x, y);
    if (plot.state != LandState::Empty && plot.state != LandState::Uncultivated) {
        result["ok"] = false; result["error"] = "当前格无法种植，请先移动到空地或开垦土地"; return result;
    }
    d->enqueuePlant(x, y, cropTypeFromName(cropName));
    result["ok"] = true;
    result["message"] = QString("已安排在 (%1,%2) 种植 %3").arg(x).arg(y).arg(cropName);
    return result;
}

QJsonObject ScriptAPI::harvest(int x, int y) {
    QJsonObject result;
    if (x < 0 || x >= m_gameState->mapWidth() || y < 0 || y >= m_gameState->mapHeight()) {
        result["ok"] = false;
        return result;
    }
    LandPlot &plot = m_gameState->plot(x, y);
    if (plot.state != LandState::Harvestable) {
        result["ok"] = false;
        result["error"] = "作物尚未成熟";
        return result;
    }
    if (!m_gameState->drones().isEmpty()) {
        m_gameState->drones().first()->enqueueTask(DroneTask::Harvest, x, y);
    }
    result["ok"] = true;
    return result;
}

QJsonObject ScriptAPI::harvestCurrent()
{
    QJsonObject result;
    if (m_gameState->drones().isEmpty()) { result["ok"] = false; result["error"] = "没有可用无人机"; return result; }
    Drone *d = m_gameState->drones().first();
    const QPointF p = d->plannedPosition();
    const int x = static_cast<int>(p.x());
    const int y = static_cast<int>(p.y());
    if (x < 0 || x >= m_gameState->mapWidth() || y < 0 || y >= m_gameState->mapHeight()) {
        result["ok"] = false; result["error"] = "无人机当前位置超出地图"; return result;
    }
    const LandPlot &plot = m_gameState->plot(x, y);
    if (!d->hasTasks() && plot.state != LandState::Harvestable) {
        result["ok"] = false;
        result["error"] = QString("作物尚未成熟，当前成长 %1%，请 wait() 后再 harvest() 或用 scan() 判断 ripe")
                              .arg(static_cast<int>(plot.progress));
        return result;
    }
    d->enqueueTask(DroneTask::Harvest, x, y);
    result["ok"] = true;
    return result;
}

QJsonObject ScriptAPI::move(int droneId, int x, int y) {
    QJsonObject result;
    Drone *d = m_gameState->drone(droneId);
    if (!d) {
        result["ok"] = false;
        result["error"] = "无人机不存在";
        return result;
    }
    d->enqueueTask(DroneTask::Move, x, y);
    result["ok"] = true;
    return result;
}

QJsonObject ScriptAPI::moveDir(int dx, int dy)
{
    QJsonObject result;
    if (m_gameState->drones().isEmpty()) { result["ok"] = false; result["error"] = "没有可用无人机"; return result; }
    Drone *d = m_gameState->drones().first();
    const QPointF p = d->plannedPosition();
    const int x = static_cast<int>(p.x()) + dx;
    const int y = static_cast<int>(p.y()) + dy;
    if (x < 0 || x >= m_gameState->mapWidth() || y < 0 || y >= m_gameState->mapHeight()) {
        result["ok"] = false; result["error"] = "无人机无法移出地图"; return result;
    }
    d->enqueueTask(DroneTask::Move, x, y);
    result["ok"] = true;
    result["x"] = x;
    result["y"] = y;
    return result;
}

QJsonObject ScriptAPI::water(int x, int y) {
    QJsonObject result;
    if (!m_gameState->drones().isEmpty()) {
        m_gameState->drones().first()->enqueueTask(DroneTask::Water, x, y);
    }
    result["ok"] = true;
    return result;
}

QJsonObject ScriptAPI::waterCurrent() { return enqueueCurrent(DroneTask::Water); }

QJsonObject ScriptAPI::fertilize(int x, int y) {
    QJsonObject result;
    if (!m_gameState->drones().isEmpty()) {
        m_gameState->drones().first()->enqueueTask(DroneTask::Fertilize, x, y);
    }
    result["ok"] = true;
    return result;
}

QJsonObject ScriptAPI::fertilizeCurrent() { return enqueueCurrent(DroneTask::Fertilize); }

QJsonObject ScriptAPI::debugPlot(int x, int y) {
    QJsonObject result;
    if (!m_gameState->drones().isEmpty()) {
        m_gameState->drones().first()->enqueueTask(DroneTask::DebugPlot, x, y);
    }
    result["ok"] = true;
    return result;
}

QJsonObject ScriptAPI::scan(int x, int y) {
    QJsonObject result;
    if (x < 0 || x >= m_gameState->mapWidth() || y < 0 || y >= m_gameState->mapHeight()) {
        result["ok"] = false;
        return result;
    }
    const LandPlot &plot = m_gameState->plot(x, y);
    result["water"] = plot.water;
    result["fertility"] = plot.fertility;
    result["crop"] = cropName(plot.crop);
    result["progress"] = plot.progress;
    result["bug"] = plot.bug;
    result["state"] = landStateName(plot.state);
    result["ripe"] = plot.state == LandState::Harvestable;
    result["ok"] = true;
    return result;
}

QJsonObject ScriptAPI::scanCurrent()
{
    if (m_gameState->drones().isEmpty()) { QJsonObject r; r["ok"] = false; r["error"] = "没有可用无人机"; return r; }
    const QPointF p = m_gameState->drones().first()->plannedPosition();
    return scan(static_cast<int>(p.x()), static_cast<int>(p.y()));
}

QJsonObject ScriptAPI::getPos()
{
    QJsonObject result;
    if (m_gameState->drones().isEmpty()) { result["ok"] = false; return result; }
    const QPointF p = m_gameState->drones().first()->plannedPosition();
    result["ok"] = true;
    result["x"] = static_cast<int>(p.x());
    result["y"] = static_cast<int>(p.y());
    return result;
}

QJsonObject ScriptAPI::tillCurrent() { return enqueueCurrent(DroneTask::Cultivate); }

QJsonObject ScriptAPI::wait(int ms)
{
    QJsonObject result;
    if (m_gameState->drones().isEmpty()) { result["ok"] = false; result["error"] = "没有可用无人机"; return result; }
    m_gameState->drones().first()->enqueueWait(qMax(0, ms));
    result["ok"] = true;
    result["message"] = QString("等待 %1 ms").arg(ms);
    return result;
}

QJsonObject ScriptAPI::enqueueCurrent(DroneTask task)
{
    QJsonObject result;
    if (m_gameState->drones().isEmpty()) { result["ok"] = false; result["error"] = "没有可用无人机"; return result; }
    Drone *d = m_gameState->drones().first();
    const QPointF p = d->plannedPosition();
    const int x = static_cast<int>(p.x());
    const int y = static_cast<int>(p.y());
    if (x < 0 || x >= m_gameState->mapWidth() || y < 0 || y >= m_gameState->mapHeight()) {
        result["ok"] = false; result["error"] = "无人机当前位置超出地图"; return result;
    }
    d->enqueueTask(task, x, y);
    result["ok"] = true;
    return result;
}

QJsonObject ScriptAPI::nearest(int droneId, const QString &stateFilter) {
    QJsonObject result;
    // Find nearest plot matching state
    Drone *d = m_gameState->drone(droneId);
    if (!d) {
        result["ok"] = false;
        return result;
    }
    LandState targetState = LandState::Harvestable;
    if (stateFilter == "empty") targetState = LandState::Empty;
    else if (stateFilter == "bugged") targetState = LandState::Bugged;
    else if (stateFilter == "planted") targetState = LandState::Planted;

    QPointF dpos = d->position();
    double bestDist = 1e18;
    int bestX = -1, bestY = -1;

    for (int y = 0; y < m_gameState->mapHeight(); ++y) {
        for (int x = 0; x < m_gameState->mapWidth(); ++x) {
            if (m_gameState->plot(x, y).state == targetState) {
                double dist = (dpos.x() - x) * (dpos.x() - x) +
                              (dpos.y() - y) * (dpos.y() - y);
                if (dist < bestDist) {
                    bestDist = dist;
                    bestX = x;
                    bestY = y;
                }
            }
        }
    }
    result["x"] = bestX;
    result["y"] = bestY;
    result["ok"] = true;
    return result;
}

QJsonObject ScriptAPI::pathfind(int x1, int y1, int x2, int y2) {
    QJsonObject result;
    int w = m_gameState->mapWidth();
    int h = m_gameState->mapHeight();
    QVector<QVector<bool>> walkable(h, QVector<bool>(w, true));
    auto path = PathFinder::findPath(x1, y1, x2, y2, w, h, walkable);
    QJsonArray arr;
    for (auto &p : path) {
        QJsonObject pt;
        pt["x"] = p.x();
        pt["y"] = p.y();
        arr.append(pt);
    }
    result["path"] = arr;
    result["ok"] = true;
    return result;
}
