#pragma once

#include "types.h"
#include "levels.h"
#include <QObject>
#include <QTimer>
#include <QPoint>
#include <random>

class GameEngine : public QObject {
    Q_OBJECT
public:
    explicit GameEngine(QObject *parent = nullptr);

    const QVector<Cell> &getCells() const { return m_cells; }
    const QVector<Goal> &getGoals() const { return m_goals; }
    QPoint getDrone() const { return m_drone; }
    QSize getGridSize() const { return m_level.size; }
    double getElapsed() const { return m_elapsed; }
    int getLevelSeconds() const { return m_level.seconds; }
    bool isRunning() const { return m_running; }
    const Level &currentLevel() const { return m_level; }
    QString goalText() const;

    void loadLevel(const Level &level);
    void run(const QString &code);
    void step(const QString &code);
    void stop();

signals:
    void logMessage(const QString &msg);
    void stateChanged();
    void levelFinished(bool success, int stars);
    void commandHighlight(int lineIndex, const QString &line);

private:
    void tick();
    Cell &current();
    void executeNext();
    void runCommand(const QString &line);
    void doMove(const QString &dir);
    void doTill();
    void doPlant(const QString &crop);
    void doWater();
    void doFertilize();
    void doDebug();
    void doHarvest();
    void describeCurrent();
    void growCrops();
    bool complete() const;
    QStringList parseCommands(const QString &code) const;

    Level m_level;
    QVector<Cell> m_cells;
    QVector<Goal> m_goals;
    QPoint m_drone{0, 0};
    QStringList m_commands;
    int m_pc = 0;
    double m_elapsed = 0;
    bool m_running = false;
    QTimer m_tickTimer;
    std::mt19937 m_rng;
};
