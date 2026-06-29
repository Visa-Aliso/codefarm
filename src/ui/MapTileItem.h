#ifndef MAPTILEITEM_H
#define MAPTILEITEM_H

#include <QGraphicsObject>
#include "core/LandPlot.h"

class GameState;

class MapTileItem : public QGraphicsObject {
    Q_OBJECT
public:
    MapTileItem(GameState *state, int gridX, int gridY,
                QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    int gridX() const { return m_gridX; }
    int gridY() const { return m_gridY; }
    void refresh();

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    void drawDiamond(QPainter *painter, const QColor &baseColor);
    void drawCrop(QPainter *painter);
    void drawOverlays(QPainter *painter);

    GameState *m_gameState;
    int m_gridX;
    int m_gridY;
    bool m_hovered = false;

    static constexpr int TILE_W = 128;
    static constexpr int TILE_H = 64;
};

#endif // MAPTILEITEM_H
