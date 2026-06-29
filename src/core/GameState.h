#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <QObject>
#include <QVector>
#include <QSet>
#include <QString>
#include <QTimer>
#include <QJsonObject>
#include <QJsonArray>
#include "core/LandPlot.h"
#include "core/Weather.h"

class Drone;
class TechTree;

class GameState : public QObject {
    Q_OBJECT
public:
    explicit GameState(QObject *parent = nullptr);
    ~GameState();

    void reset();
    void resetFarmRuntime();
    void startTicking();
    void stopTicking();
    bool isTicking() const { return m_ticking; }

    // Accessors
    int gold() const { return m_gold; }
    void addGold(int amount);
    bool spendGold(int amount);

    int level() const { return m_level; }
    int mapWidth() const { return m_mapWidth; }
    int mapHeight() const { return m_mapHeight; }
    void setMapSize(int w, int h);

    Weather *weather() { return &m_weather; }
    const Weather *weather() const { return &m_weather; }

    TechTree *techTree() { return m_techTree; }
    const TechTree *techTree() const { return m_techTree; }

    // Plots
    LandPlot &plot(int x, int y);
    const LandPlot &plot(int x, int y) const;
    QVector<LandPlot> &plots() { return m_plots; }
    const QVector<LandPlot> &plots() const { return m_plots; }
    int plotIndex(int x, int y) const { return y * m_mapWidth + x; }

    // Drones
    QVector<Drone *> &drones() { return m_drones; }
    const QVector<Drone *> &drones() const { return m_drones; }
    Drone *drone(int id);

    // Unlocked functions/syntax
    QSet<QString> unlockedFunctions() const { return m_unlockedFunctions; }
    void unlockFunction(const QString &name);
    bool isFunctionUnlocked(const QString &name) const;

    QSet<QString> unlockedSyntax() const { return m_unlockedSyntax; }
    void unlockSyntax(const QString &name);
    bool isSyntaxUnlocked(const QString &name) const;
    void applyTechNode(const QString &id);

    // Scripts
    struct ScriptData {
        QString name;
        QString code;
    };
    QVector<ScriptData> &scripts() { return m_scripts; }
    const QVector<ScriptData> &scripts() const { return m_scripts; }

    // Serialization
    bool saveToFile(const QString &path, const QByteArray &windowLayout) const;
    bool loadFromFile(const QString &path);
    QByteArray savedWindowLayout() const { return m_savedWindowLayout; }

signals:
    void goldChanged(int gold);
    void levelChanged(int level);
    void mapSizeChanged(int w, int h);
    void plotChanged(int x, int y);
    void droneChanged(int droneId);
    void harvestCompleted(int x, int y, int goldEarned);
    void droneTaskCompleted(int droneId, int x, int y);

public slots:
    void tick();

private:
    void updateLevel();
    void updatePlots(double dt);
    void updateDrones(double dt);
    void processDroneTask(Drone *d, double dt);
    int getLevelForMapSize(int w, int h) const;

    int m_gold = 100;
    int m_level = 1;
    int m_mapWidth = 1;
    int m_mapHeight = 1;
    bool m_ticking = false;
    Weather m_weather;
    TechTree *m_techTree;
    QVector<LandPlot> m_plots;
    QVector<Drone *> m_drones;
    QSet<QString> m_unlockedFunctions;
    QSet<QString> m_unlockedSyntax;
    QVector<ScriptData> m_scripts;
    QByteArray m_savedWindowLayout;
    QTimer m_tickTimer;
};

#endif // GAMESTATE_H
