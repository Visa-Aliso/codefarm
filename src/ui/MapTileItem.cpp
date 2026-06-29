#include "MapTileItem.h"
#include "core/GameState.h"
#include "core/CropData.h"
#include <QPainter>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QRadialGradient>
#include <QCursor>

MapTileItem::MapTileItem(GameState *state, int gridX, int gridY,
                         QGraphicsItem *parent)
    : QGraphicsObject(parent)
    , m_gameState(state)
    , m_gridX(gridX)
    , m_gridY(gridY)
{
    setAcceptHoverEvents(true);
    setCursor(Qt::PointingHandCursor);
}

QRectF MapTileItem::boundingRect() const {
    return QRectF(-TILE_W / 2.0, -TILE_H / 2.0, TILE_W, TILE_H);
}

void MapTileItem::paint(QPainter *painter,
                         const QStyleOptionGraphicsItem *,
                         QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing);

    const LandPlot &plot = m_gameState->plot(m_gridX, m_gridY);

    // Base color by state
    QColor baseColor("#D4A574");
    switch (plot.state) {
    case LandState::Uncultivated:
        baseColor = QColor("#A0937D");
        break;
    case LandState::Empty:
        baseColor = QColor("#D4A574");
        break;
    case LandState::Planted:
    case LandState::Mature:
        baseColor = QColor("#B8A57A");
        break;
    case LandState::Harvestable:
        baseColor = QColor("#C4B87A");
        break;
    case LandState::Wasteland:
        baseColor = QColor("#9B8B7A");
        break;
    case LandState::Bugged:
        baseColor = QColor("#B09080");
        break;
    }

    drawDiamond(painter, baseColor);
    drawCrop(painter);
    drawOverlays(painter);
}

void MapTileItem::drawDiamond(QPainter *painter, const QColor &baseColor) {
    QPolygonF diamond;
    diamond << QPointF(0, -TILE_H / 2.0)
            << QPointF(TILE_W / 2.0, 0)
            << QPointF(0, TILE_H / 2.0)
            << QPointF(-TILE_W / 2.0, 0);

    QLinearGradient grad(QPointF(-TILE_W / 2, 0), QPointF(TILE_W / 2, 0));
    grad.setColorAt(0.0, m_hovered ? baseColor.lighter(110) : baseColor.darker(105));
    grad.setColorAt(0.5, m_hovered ? baseColor.lighter(120) : baseColor);
    grad.setColorAt(1.0, m_hovered ? baseColor.lighter(110) : baseColor.darker(105));

    painter->setPen(QPen(QColor("#8B7355"), 1.5));
    painter->setBrush(grad);
    painter->drawPolygon(diamond);

    // Selection / hover highlight
    if (m_hovered) {
        painter->setPen(QPen(QColor("#7EC8E3"), 2.5));
        painter->setBrush(Qt::NoBrush);
        painter->drawPolygon(diamond);
    }
}

void MapTileItem::drawCrop(QPainter *painter) {
    const LandPlot &plot = m_gameState->plot(m_gridX, m_gridY);
    if (plot.crop == CropType::None) return;

    const CropDef &def = CropData::get(plot.crop);
    Q_UNUSED(def);
    int stage = 0;
    if (plot.progress > 75) stage = 3;
    else if (plot.progress > 50) stage = 2;
    else if (plot.progress > 25) stage = 1;

    painter->setPen(Qt::NoPen);

    const QPointF base(0, -TILE_H / 4.0);
    const double scale = 0.65 + stage * 0.16;

    painter->setPen(QPen(QColor("#5D7545"), 2, Qt::SolidLine, Qt::RoundCap));
    painter->drawLine(base, base + QPointF(0, -20 * scale));

    painter->setPen(Qt::NoPen);
    painter->setBrush(plot.bug ? QColor("#B99C8A") : QColor("#78A864"));
    painter->drawEllipse(base + QPointF(-10 * scale, -13 * scale), 9 * scale, 5 * scale);
    painter->drawEllipse(base + QPointF(10 * scale, -15 * scale), 9 * scale, 5 * scale);

    if (plot.crop == CropType::Apple) {
        painter->setBrush(QColor("#DCEBC5"));
        painter->drawEllipse(base + QPointF(0, -21 * scale), 13 * scale, 8 * scale);
        if (stage >= 2) {
            painter->setBrush(QColor("#D95D5D"));
            painter->drawEllipse(base + QPointF(-5 * scale, -26 * scale), 4.5 * scale, 5.5 * scale);
            painter->drawEllipse(base + QPointF(5 * scale, -22 * scale), 4.5 * scale, 5.5 * scale);
        }
    } else if (plot.crop == CropType::Grape) {
        painter->setBrush(QColor("#7AA35F"));
        painter->drawRoundedRect(QRectF(base.x() - 14 * scale, base.y() - 30 * scale,
                                        28 * scale, 14 * scale), 7, 7);
        if (stage >= 2) {
            painter->setBrush(QColor("#E36F54"));
            painter->drawEllipse(base + QPointF(-7 * scale, -24 * scale), 5 * scale, 5 * scale);
            painter->drawEllipse(base + QPointF(7 * scale, -25 * scale), 5 * scale, 5 * scale);
        }
    } else if (plot.crop == CropType::Banana) {
        painter->setPen(QPen(QColor("#7B6AA8"), 2, Qt::SolidLine, Qt::RoundCap));
        painter->drawLine(base + QPointF(-13 * scale, -23 * scale), base + QPointF(13 * scale, -23 * scale));
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor("#8E77B5"));
        if (stage >= 2) {
            for (int i = 0; i < 5; ++i)
                painter->drawEllipse(base + QPointF((-8 + i * 4) * scale, (-27 + (i % 2) * 5) * scale),
                                     3.2 * scale, 4.2 * scale);
        }
    } else if (plot.crop == CropType::Watermelon) {
        painter->setBrush(QColor("#7BBF74"));
        painter->drawEllipse(base + QPointF(0, -17 * scale), 15 * scale, 9 * scale);
        if (stage >= 2) {
            painter->setPen(QPen(QColor("#477B4C"), 1));
            painter->drawArc(QRectF(base.x() - 11 * scale, base.y() - 26 * scale,
                                    22 * scale, 18 * scale), 20 * 16, 140 * 16);
            painter->setPen(Qt::NoPen);
        }
    } else if (plot.crop == CropType::Pineapple) {
        painter->setBrush(QColor("#E3B75F"));
        painter->drawRoundedRect(QRectF(base.x() - 8 * scale, base.y() - 30 * scale,
                                        16 * scale, 18 * scale), 7, 7);
        painter->setBrush(QColor("#6FA45F"));
        painter->drawPolygon(QPolygonF() << base + QPointF(0, -44 * scale)
                                         << base + QPointF(7 * scale, -30 * scale)
                                         << base + QPointF(-7 * scale, -30 * scale));
    }

    if (plot.bug) {
        painter->setPen(QPen(QColor("#D04040"), 2));
        painter->drawLine(base + QPointF(-8, -35), base + QPointF(8, -45));
        painter->drawLine(base + QPointF(8, -35), base + QPointF(-8, -45));
    }
}

void MapTileItem::drawOverlays(QPainter *painter) {
    const LandPlot &plot = m_gameState->plot(m_gridX, m_gridY);

    // Harvestable glow
    if (plot.state == LandState::Harvestable) {
        QRadialGradient glow(QPointF(0, 0), TILE_W / 2.0);
        glow.setColorAt(0.2, QColor(232, 200, 106, 60));
        glow.setColorAt(0.8, QColor(232, 200, 106, 0));
        painter->setPen(Qt::NoPen);
        painter->setBrush(glow);
        painter->drawEllipse(QPointF(0, 0), TILE_W / 2.0, TILE_H / 2.0);
    }
}

void MapTileItem::refresh() {
    update();
}

void MapTileItem::hoverEnterEvent(QGraphicsSceneHoverEvent *) {
    m_hovered = true;
    update();
    // CropDetailPopup shown here when implemented
}

void MapTileItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *) {
    m_hovered = false;
    update();
}

void MapTileItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsObject::mousePressEvent(event);
    // Context actions on click (plant, harvest, etc.)
    // Click handling via scene event filter in MapView
}
