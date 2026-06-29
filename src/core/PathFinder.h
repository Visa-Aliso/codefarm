#ifndef PATHFINDER_H
#define PATHFINDER_H

#include <QVector>
#include <QPoint>

class PathFinder {
public:
    // A* on grid, returns path from (x1,y1) to (x2,y2) excluding start
    static QVector<QPoint> findPath(int x1, int y1, int x2, int y2,
                                     int mapWidth, int mapHeight,
                                     const QVector<QVector<bool>> &walkable);
};

#endif // PATHFINDER_H
