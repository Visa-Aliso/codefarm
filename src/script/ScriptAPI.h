#ifndef SCRIPTAPI_H
#define SCRIPTAPI_H

#include <QObject>
#include <QJsonObject>
#include "core/Drone.h"

class GameState;

// Game functions exposed to Python scripts
class ScriptAPI : public QObject {
    Q_OBJECT
public:
    explicit ScriptAPI(GameState *state, QObject *parent = nullptr);

    // Callable from Python
    QJsonObject init();
    QJsonObject plant(int x, int y);
    QJsonObject plantCurrent(const QString &cropName = QStringLiteral("strawberry"));
    QJsonObject harvest(int x, int y);
    QJsonObject harvestCurrent();
    QJsonObject move(int droneId, int x, int y);
    QJsonObject moveDir(int dx, int dy);
    QJsonObject water(int x, int y);
    QJsonObject waterCurrent();
    QJsonObject fertilize(int x, int y);
    QJsonObject fertilizeCurrent();
    QJsonObject debugPlot(int x, int y);
    QJsonObject scan(int x, int y);
    QJsonObject scanCurrent();
    QJsonObject getPos();
    QJsonObject tillCurrent();
    QJsonObject wait(int ms);
    QJsonObject nearest(int droneId, const QString &stateFilter);
    QJsonObject pathfind(int x1, int y1, int x2, int y2);

private:
    GameState *m_gameState;
    QJsonObject enqueueCurrent(DroneTask task);
    int cropTypeFromName(const QString &name) const;
};

#endif // SCRIPTAPI_H
