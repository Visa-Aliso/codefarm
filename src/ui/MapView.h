#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QGraphicsView>

class GameState;
class QGraphicsScene;
class MapTileItem;
class DroneItem;
class CropDetailPopup;

class MapView : public QGraphicsView {
    Q_OBJECT
public:
    explicit MapView(GameState *state, QWidget *parent = nullptr);

    void rebuildScene();
    void resetViewTransform();
    GameState *gameState() const { return m_gameState; }

    // Coordinate transforms
    static QPointF gridToScene(int x, int y);
    static QPoint sceneToGrid(QPointF pos);

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    void setupScene();
    void buildTiles();
    void buildDrones();
    QPointF droneScenePos(const QPointF &gridPos) const;

    GameState *m_gameState;
    QGraphicsScene *m_scene;
    QVector<MapTileItem *> m_tileItems;
    QVector<DroneItem *> m_droneItems;
    CropDetailPopup *m_detailPopup;
    bool m_panning = false;
    QPoint m_lastPanPoint;
    QPointF m_lastPanScene;
    double m_zoom = 1.0;

    // Tile geometry
    static constexpr int TILE_W = 128;
    static constexpr int TILE_H = 64;
};

#endif // MAPVIEW_H
