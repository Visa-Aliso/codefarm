#include "GameState.h"
#include "core/Drone.h"
#include "core/TechTree.h"
#include "core/CropData.h"
#include "core/PathFinder.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QDebug>
#include <QRandomGenerator>
#include <algorithm>
#include <cmath>

GameState::GameState(QObject *parent)
    : QObject(parent)
{
    m_techTree = new TechTree(this);
    connect(&m_tickTimer, &QTimer::timeout, this, &GameState::tick);
    reset();
}

GameState::~GameState()
{
    stopTicking();
}

void GameState::reset()
{
    m_gold = 80;
    m_level = 1;
    m_mapWidth = 1;
    m_mapHeight = 1;
    m_weather.setWeather(WeatherType::Sunny);

    m_plots.clear();
    m_plots.resize(m_mapWidth * m_mapHeight);
    for (auto &p : m_plots)
        p = LandPlot();
    // Start with poor but cultivatable soil; automation must improve it.
    m_plots[0].state = LandState::Empty;
    m_plots[0].water = 22;
    m_plots[0].fertility = 12;

    qDeleteAll(m_drones);
    m_drones.clear();

    auto *d0 = new Drone(0, this);
    d0->setPosition(QPointF(0, 0));
    m_drones.append(d0);

    m_unlockedFunctions.clear();
    m_unlockedFunctions.insert("plant");
    m_unlockedFunctions.insert("harvest");
    m_unlockedFunctions.insert("wait");
    m_unlockedFunctions.insert("till");
    m_unlockedFunctions.insert("init");

    m_unlockedSyntax.clear();

    m_scripts.clear();

    m_techTree->setUnlocked({"A0", "B0", "C0", "D0"});

    m_savedWindowLayout.clear();
}

void GameState::resetFarmRuntime()
{
    for (auto &p : m_plots)
        p = LandPlot();

    for (int y = 0; y < m_mapHeight; ++y) {
        for (int x = 0; x < m_mapWidth; ++x) {
            LandPlot &p = plot(x, y);
            p.state = (x == 0 && y == 0) ? LandState::Empty : LandState::Uncultivated;
            p.water = (x == 0 && y == 0) ? 22 : 12;
            p.fertility = (x == 0 && y == 0) ? 12 : 8;
            p.crop = CropType::None;
            p.progress = 0;
            p.bug = false;
            emit plotChanged(x, y);
        }
    }

    for (auto *d : m_drones) {
        d->clearTasks();
        d->setPosition(QPointF(0, 0));
        d->setState(DroneState::Idle);
        d->resetWorkTimer();
        emit droneChanged(d->id());
    }
}

void GameState::startTicking() {
    m_ticking = true;
    m_tickTimer.start(100); // 100ms = 10 ticks/sec
}

void GameState::stopTicking() {
    m_ticking = false;
    m_tickTimer.stop();
}

void GameState::tick() {
    double dt = 0.1; // 100ms in seconds

    // 1. Weather may transition (handled by Weather's own timer)
    // 2. Update plots
    updatePlots(dt);
    // 3. Update drones
    updateDrones(dt);
}

void GameState::updatePlots(double dt) {
    double waterDrainBase = 0.08; // base water loss per second
    double waterDrain = waterDrainBase * m_weather.waterDrainMultiplier();
    double fertDrainBase = 0.02;
    double bugChanceBase = 0.001; // base chance per tick

    for (int y = 0; y < m_mapHeight; ++y) {
        for (int x = 0; x < m_mapWidth; ++x) {
            int idx = plotIndex(x, y);
            LandPlot &p = m_plots[idx];

            if (p.state == LandState::Uncultivated || p.state == LandState::Wasteland)
                continue;

            double cropWaterUse = 0.0;
            double cropFertUse = 0.0;
            if (p.crop != CropType::None) {
                const CropDef &crop = CropData::get(p.crop);
                cropWaterUse = crop.waterUsePerSec;
                cropFertUse = crop.fertUsePerSec;
            }

            p.water -= (waterDrain + cropWaterUse) * dt;
            if (p.water < 0) p.water = 0;
            // Rainy weather: auto-water
            if (m_weather.current() == WeatherType::Rainy)
                p.water += 1.0 * dt;
            if (p.water > 100) p.water = 100;

            // Fertility drain
            if (p.state == LandState::Planted || p.state == LandState::Mature)
                p.fertility -= (fertDrainBase + cropFertUse) * dt;
            if (p.fertility < 0) p.fertility = 0;

            // Growth
            if (p.state == LandState::Planted && p.water > 0) {
                const CropDef &crop = CropData::get(p.crop);
                double fertilityFactor = 0.55 + (p.fertility / 160.0);
                double weatherFactor = m_weather.growthMultiplier();
                double growth = (100.0 / crop.growthTime) * fertilityFactor * weatherFactor * dt;
                p.progress += growth;
                if (p.progress >= 100.0) {
                    p.progress = 100.0;
                    p.state = LandState::Harvestable;
                }
                emit plotChanged(x, y);
            }

            // Bug check
            if (p.state == LandState::Planted || p.state == LandState::Mature ||
                p.state == LandState::Harvestable) {
                if (!p.bug) {
                    const CropDef &crop = CropData::get(p.crop);
                    double resist = crop.bugResistance + m_techTree->diseaseResistBonus();
                    double chance = bugChanceBase * (1.0 - resist) + m_weather.bugChanceBonus() * 0.01;
                    if (QRandomGenerator::global()->bounded(1.0) < chance) {
                        p.bug = true;
                        emit plotChanged(x, y);
                    }
                }
            }
        }
    }
}

void GameState::updateDrones(double dt) {
    for (auto *d : m_drones) {
        if (d->state() == DroneState::Idle && d->hasTasks()) {
            // Start next task
            d->setState(DroneState::Moving);
            emit droneChanged(d->id());
        }
        processDroneTask(d, dt);
    }
}

void GameState::processDroneTask(Drone *d, double dt) {
    if (d->state() == DroneState::Idle) return;

    DroneTaskItem task = d->currentTask();
    double droneSpeed = d->speed() * m_weather.droneSpeedMultiplier();
    double moveStep = droneSpeed * 60.0 * dt; // pixels per tick at base speed

    QPointF targetPos(task.targetX, task.targetY);
    QPointF current = d->position();
    double dx = targetPos.x() - current.x();
    double dy = targetPos.y() - current.y();
    double dist = std::sqrt(dx * dx + dy * dy);

    if (d->state() == DroneState::Moving) {
        if (dist < 0.3) {
            // Reached target, start working
            d->setPosition(targetPos);
            d->setState(DroneState::Working);
            d->resetWorkTimer();
            emit droneChanged(d->id());
        } else {
            // Move toward target
            double ratio = moveStep / dist;
            if (ratio > 1.0) ratio = 1.0;
            d->setPosition(QPointF(current.x() + dx * ratio, current.y() + dy * ratio));
            emit droneChanged(d->id());
        }
        return;
    }

    if (d->state() == DroneState::Working) {
        double actionTime = task.task == DroneTask::Wait
                                ? qMax(0.0, task.waitMs / 1000.0)
                                : 0.3 / d->actionSpeed();
        d->addWorkTime(dt);
        if (d->workTimer() < actionTime) return;
        d->resetWorkTimer();

        int tx = task.targetX;
        int ty = task.targetY;

        if (tx >= 0 && tx < m_mapWidth && ty >= 0 && ty < m_mapHeight) {
            LandPlot &plot = this->plot(tx, ty);

            switch (task.task) {
            case DroneTask::Plant:
                if (plot.state == LandState::Empty || plot.state == LandState::Uncultivated) {
                    plot.crop = static_cast<CropType>(task.cropType);
                    plot.progress = 0;
                    plot.state = LandState::Planted;
                    emit plotChanged(tx, ty);
                }
                break;
            case DroneTask::Harvest:
                if (plot.state == LandState::Harvestable) {
                    const CropDef &crop = CropData::get(plot.crop);
                    double quality = 0.70 + std::min(0.30, plot.fertility / 260.0);
                    int gold = static_cast<int>(crop.yield * quality * m_techTree->yieldMultiplier());
                    addGold(gold);
                    if (crop.replantAfterHarvest) {
                        plot.crop = CropType::None;
                        plot.progress = 0;
                        plot.state = LandState::Empty;
                    } else {
                        plot.progress = 0;
                        plot.state = LandState::Planted;
                    }
                    emit harvestCompleted(tx, ty, gold);
                    emit plotChanged(tx, ty);
                }
                break;
            case DroneTask::Water:
                plot.water = std::min(100.0, plot.water + 30);
                emit plotChanged(tx, ty);
                break;
            case DroneTask::Fertilize:
                plot.fertility = std::min(100.0 + m_techTree->fertilityCapBonus(), plot.fertility + 25);
                emit plotChanged(tx, ty);
                break;
            case DroneTask::DebugPlot:
                plot.bug = false;
                emit plotChanged(tx, ty);
                break;
            case DroneTask::Cultivate:
                if (plot.state == LandState::Uncultivated || plot.state == LandState::Wasteland) {
                    plot.state = LandState::Empty;
                    plot.water = 100;
                    plot.fertility = 100;
                    emit plotChanged(tx, ty);
                }
                break;
            case DroneTask::Move:
                // Already at target, nothing to do
                break;
            case DroneTask::Wait:
                break;
            }
        }

        // Task complete
        d->dequeueTask();
        emit droneTaskCompleted(d->id(), tx, ty);

        if (d->hasTasks())
            d->setState(DroneState::Moving);
        else
            d->setState(DroneState::Idle);
        emit droneChanged(d->id());
    }
}

void GameState::addGold(int amount) {
    m_gold += amount;
    emit goldChanged(m_gold);
}

bool GameState::spendGold(int amount) {
    if (m_gold < amount) return false;
    m_gold -= amount;
    emit goldChanged(m_gold);
    return true;
}

void GameState::setMapSize(int w, int h) {
    if (w == m_mapWidth && h == m_mapHeight) return;

    QVector<LandPlot> newPlots(w * h);
    // Copy existing plots where they overlap
    for (int y = 0; y < h && y < m_mapHeight; ++y)
        for (int x = 0; x < w && x < m_mapWidth; ++x)
            newPlots[y * w + x] = m_plots[y * m_mapWidth + x];

    // New plots default to Uncultivated
    for (int i = 0; i < newPlots.size(); ++i) {
        if (newPlots[i].state == LandState::Uncultivated)
            ; // already default
    }

    m_mapWidth = w;
    m_mapHeight = h;
    m_plots = std::move(newPlots);

    updateLevel();
    emit mapSizeChanged(w, h);
}

int GameState::getLevelForMapSize(int w, int h) const {
    int maxDim = std::max(w, h);
    if (maxDim >= 50) return 8;
    if (maxDim >= 25) return 7;
    if (maxDim >= 15) return 6;
    if (maxDim >= 8) return 5;
    if (maxDim >= 5) return 4;
    if (maxDim >= 3) return 3;
    if (maxDim >= 2) return 2;
    return 1;
}

void GameState::updateLevel() {
    int newLevel = getLevelForMapSize(m_mapWidth, m_mapHeight);
    if (newLevel != m_level) {
        m_level = newLevel;
        emit levelChanged(m_level);
    }
}

LandPlot &GameState::plot(int x, int y) {
    return m_plots[plotIndex(x, y)];
}

const LandPlot &GameState::plot(int x, int y) const {
    return m_plots[plotIndex(x, y)];
}

Drone *GameState::drone(int id) {
    for (auto *d : m_drones)
        if (d->id() == id) return d;
    return nullptr;
}

void GameState::unlockFunction(const QString &name) {
    m_unlockedFunctions.insert(name);
}

bool GameState::isFunctionUnlocked(const QString &name) const {
    return m_unlockedFunctions.contains(name);
}

void GameState::unlockSyntax(const QString &name) {
    m_unlockedSyntax.insert(name);
}

bool GameState::isSyntaxUnlocked(const QString &name) const {
    return m_unlockedSyntax.contains(name);
}

void GameState::applyTechNode(const QString &id)
{
    if (id == "B1") {
        for (auto *d : m_drones)
            d->setSpeedMultiplier(1.0 + m_techTree->droneSpeedBonus());
        return;
    }

    if (id == "B2" || id == "B4") {
        const int targetCount = 1 + m_techTree->extraDrones();
        while (m_drones.size() < targetCount) {
            auto *d = new Drone(m_drones.size(), this);
            d->setPosition(QPointF(0, 0));
            d->setSpeedMultiplier(1.0 + m_techTree->droneSpeedBonus());
            m_drones.append(d);
            emit droneChanged(d->id());
        }
        return;
    }

    if (id == "B3") {
        unlockFunction("pathfind");
        return;
    }

    if (id == "C1") { unlockFunction("move"); unlockFunction("move_up"); unlockFunction("move_down"); unlockFunction("move_left"); unlockFunction("move_right"); unlockFunction("get_pos"); }
    else if (id == "C2") { unlockFunction("water"); unlockFunction("fertilize"); unlockFunction("debug"); }
    else if (id == "C3") { unlockSyntax("if"); unlockSyntax("else"); unlockFunction("scan"); }
    else if (id == "C4") { unlockSyntax("for"); unlockSyntax("range"); unlockFunction("nearest"); }
    else if (id == "C5") { unlockSyntax("while"); unlockSyntax("break"); unlockSyntax("continue"); }
    else if (id == "C6") { unlockSyntax("def"); unlockSyntax("return"); }
    else if (id == "C7") { unlockSyntax("list"); unlockSyntax("dict"); }
    else if (id == "C8") { unlockFunction("pathfind"); unlockFunction("schedule"); }

    if (id == "D1") setMapSize(2, 2);
    else if (id == "D2") setMapSize(3, 3);
    else if (id == "D3") setMapSize(5, 5);
    else if (id == "D6") setMapSize(8, 8);
    else if (id == "D7") setMapSize(15, 15);
    else if (id == "D8") setMapSize(25, 25);
    else if (id == "D9") setMapSize(50, 50);
}

// JSON Serialization
bool GameState::saveToFile(const QString &path, const QByteArray &windowLayout) const
{
    QJsonObject root;
    root["version"] = 1;
    root["gold"] = m_gold;
    root["level"] = m_level;
    root["mapWidth"] = m_mapWidth;
    root["mapHeight"] = m_mapHeight;
    root["weather"] = static_cast<int>(m_weather.current());

    QJsonArray plotsArr;
    for (auto &p : m_plots) {
        QJsonObject po;
        po["water"] = p.water;
        po["fertility"] = p.fertility;
        po["crop"] = static_cast<int>(p.crop);
        po["progress"] = p.progress;
        po["bug"] = p.bug;
        po["state"] = static_cast<int>(p.state);
        plotsArr.append(po);
    }
    root["plots"] = plotsArr;

    QJsonArray dronesArr;
    for (auto *d : m_drones)
        dronesArr.append(d->toJson());
    root["drones"] = dronesArr;

    QJsonArray funcArr;
    for (auto &f : m_unlockedFunctions)
        funcArr.append(f);
    root["unlockedFunctions"] = funcArr;

    QJsonArray syntaxArr;
    for (auto &s : m_unlockedSyntax)
        syntaxArr.append(s);
    root["unlockedSyntax"] = syntaxArr;

    QJsonArray techArr;
    for (auto &t : m_techTree->unlockedIds())
        techArr.append(t);
    root["unlockedTechNodes"] = techArr;

    QJsonArray scriptsArr;
    for (auto &s : m_scripts) {
        QJsonObject so;
        so["name"] = s.name;
        so["code"] = s.code;
        scriptsArr.append(so);
    }
    root["scripts"] = scriptsArr;

    root["windowLayout"] = QString::fromLatin1(windowLayout.toBase64());

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) return false;
    file.write(QJsonDocument(root).toJson());
    file.close();
    return true;
}

bool GameState::loadFromFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return false;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject()) return false;
    QJsonObject root = doc.object();

    reset();

    m_gold = root["gold"].toInt(100);
    m_level = root["level"].toInt(1);
    m_mapWidth = root["mapWidth"].toInt(1);
    m_mapHeight = root["mapHeight"].toInt(1);
    m_weather.setWeather(static_cast<WeatherType>(root["weather"].toInt()));

    QJsonArray plotsArr = root["plots"].toArray();
    m_plots.resize(m_mapWidth * m_mapHeight);
    for (int i = 0; i < plotsArr.size() && i < m_plots.size(); ++i) {
        QJsonObject po = plotsArr[i].toObject();
        m_plots[i].water = po["water"].toDouble(100.0);
        m_plots[i].fertility = po["fertility"].toDouble(100.0);
        m_plots[i].crop = static_cast<CropType>(po["crop"].toInt(-1));
        m_plots[i].progress = po["progress"].toDouble(0.0);
        m_plots[i].bug = po["bug"].toBool(false);
        m_plots[i].state = static_cast<LandState>(po["state"].toInt());
    }

    qDeleteAll(m_drones);
    m_drones.clear();
    QJsonArray dronesArr = root["drones"].toArray();
    for (auto val : dronesArr)
        m_drones.append(Drone::fromJson(val.toObject(), this));

    m_unlockedFunctions.clear();
    for (auto val : root["unlockedFunctions"].toArray())
        m_unlockedFunctions.insert(val.toString());

    m_unlockedSyntax.clear();
    for (auto val : root["unlockedSyntax"].toArray())
        m_unlockedSyntax.insert(val.toString());

    QSet<QString> tech;
    for (auto val : root["unlockedTechNodes"].toArray())
        tech.insert(val.toString());
    tech.insert("A0");
    tech.insert("B0");
    tech.insert("C0");
    tech.insert("D0");
    m_techTree->setUnlocked(tech);

    m_scripts.clear();
    for (auto val : root["scripts"].toArray()) {
        QJsonObject so = val.toObject();
        m_scripts.append({so["name"].toString(), so["code"].toString()});
    }

    QString layoutB64 = root["windowLayout"].toString();
    if (!layoutB64.isEmpty())
        m_savedWindowLayout = QByteArray::fromBase64(layoutB64.toLatin1());
    else
        m_savedWindowLayout.clear();

    return true;
}
