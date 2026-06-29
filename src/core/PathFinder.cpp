#include "PathFinder.h"
#include <QMap>
#include <QSet>
#include <QQueue>
#include <cmath>
#include <algorithm>

struct AStarNode {
    int x, y;
    double g, h;
    AStarNode *parent = nullptr;
    double f() const { return g + h; }
};

static double heuristic(int x1, int y1, int x2, int y2) {
    return std::sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

static QString key(int x, int y) { return QString("%1,%2").arg(x).arg(y); }

QVector<QPoint> PathFinder::findPath(int x1, int y1, int x2, int y2,
                                      int mapWidth, int mapHeight,
                                      const QVector<QVector<bool>> &walkable)
{
    QVector<QPoint> path;
    if (x1 == x2 && y1 == y2) return path;
    if (x2 < 0 || x2 >= mapWidth || y2 < 0 || y2 >= mapHeight || !walkable[y2][x2])
        return path;

    // Using QMap as a simple priority queue substitute
    QVector<AStarNode *> allNodes;
    QMap<QString, AStarNode *> openSet;
    QSet<QString> closedSet;

    auto *start = new AStarNode{x1, y1, 0, heuristic(x1, y1, x2, y2), nullptr};
    openSet[key(x1, y1)] = start;
    allNodes.append(start);

    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};

    while (!openSet.isEmpty()) {
        // Find lowest f()
        QString bestKey;
        double bestF = 1e18;
        for (auto it = openSet.begin(); it != openSet.end(); ++it) {
            if (it.value()->f() < bestF) {
                bestF = it.value()->f();
                bestKey = it.key();
            }
        }

        AStarNode *current = openSet.take(bestKey);
        if (current->x == x2 && current->y == y2) {
            // Reconstruct path
            AStarNode *p = current;
            QVector<QPoint> rev;
            while (p && p->parent) {
                rev.append(QPoint(p->x, p->y));
                p = p->parent;
            }
            for (int i = rev.size() - 1; i >= 0; --i)
                path.append(rev[i]);

            qDeleteAll(allNodes);
            return path;
        }

        closedSet.insert(bestKey);

        for (int d = 0; d < 4; ++d) {
            int nx = current->x + dx[d];
            int ny = current->y + dy[d];
            if (nx < 0 || nx >= mapWidth || ny < 0 || ny >= mapHeight) continue;
            if (!walkable[ny][nx]) continue;

            QString nk = key(nx, ny);
            if (closedSet.contains(nk)) continue;

            double ng = current->g + 1.0;
            auto *neighbor = new AStarNode{nx, ny, ng, heuristic(nx, ny, x2, y2), current};
            allNodes.append(neighbor);

            auto existing = openSet.find(nk);
            if (existing != openSet.end()) {
                if (ng < existing.value()->g) {
                    delete existing.value();
                    openSet[nk] = neighbor;
                } else {
                    delete neighbor;
                }
            } else {
                openSet[nk] = neighbor;
            }
        }
    }

    qDeleteAll(allNodes);
    return path; // no path found
}
