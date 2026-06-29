#include "MapView.h"
#include "MapTileItem.h"
#include "DroneItem.h"
#include "CropDetailPopup.h"
#include "core/GameState.h"
#include "core/Drone.h"
#include "core/CropData.h"
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QScrollBar>
#include <QWheelEvent>
#include <cmath>

MapView::MapView(GameState *state, QWidget *parent)
    : QGraphicsView(parent)
    , m_gameState(state)
    , m_detailPopup(nullptr)
{
    setupScene();

    connect(m_gameState, &GameState::plotChanged, this, [this](int x, int y) {
        const int idx = m_gameState->plotIndex(x, y);
        if (idx >= 0 && idx < m_tileItems.size())
            m_tileItems[idx]->refresh();
    });
    connect(m_gameState, &GameState::droneChanged, this, [this](int droneId) {
        if (droneId < 0 || droneId >= m_droneItems.size())
            return;
        if (auto *drone = m_gameState->drone(droneId)) {
            m_droneItems[droneId]->setPos(droneScenePos(drone->position()) + QPointF(0, -28));
            m_droneItems[droneId]->update();
        }
    });
}

void MapView::setupScene()
{
    m_scene = new QGraphicsScene(this);
    m_scene->setBackgroundBrush(QColor("#C4DFE6"));
    setScene(m_scene);
    setRenderHint(QPainter::Antialiasing);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setDragMode(QGraphicsView::NoDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    setMouseTracking(true);
    viewport()->setMouseTracking(true);

    m_detailPopup = new CropDetailPopup(this);
}

void MapView::rebuildScene()
{
    m_scene->clear();
    m_tileItems.clear();
    m_droneItems.clear();
    buildTiles();
    buildDrones();
    m_scene->setSceneRect(m_scene->itemsBoundingRect().adjusted(-600, -400, 600, 500));
}

void MapView::resetViewTransform()
{
    m_zoom = 1.0;
    resetTransform();
    centerOn(m_scene->itemsBoundingRect().center());
}

void MapView::buildTiles()
{
    int w = m_gameState->mapWidth();
    int h = m_gameState->mapHeight();

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            auto *item = new MapTileItem(m_gameState, x, y);
            item->setPos(gridToScene(x, y));
            // Z-order: back-to-front (higher x+y = closer)
            item->setZValue(x + y);
            m_scene->addItem(item);
            m_tileItems.append(item);
        }
    }
}

void MapView::buildDrones()
{
    for (auto *drone : m_gameState->drones()) {
        auto *item = new DroneItem(drone);
        item->setPos(droneScenePos(drone->position()) + QPointF(0, -28));
        item->setZValue(1000 + drone->id());
        m_scene->addItem(item);
        m_droneItems.append(item);
    }
}

QPointF MapView::gridToScene(int x, int y) {
    return QPointF((x - y) * TILE_W / 2.0, (x + y) * TILE_H / 2.0);
}

QPoint MapView::sceneToGrid(QPointF pos) {
    double fx = pos.x() / (TILE_W / 2.0);
    double fy = pos.y() / (TILE_H / 2.0);
    int gx = static_cast<int>(std::round((fx + fy) / 2.0));
    int gy = static_cast<int>(std::round((fy - fx) / 2.0));
    return QPoint(gx, gy);
}

QPointF MapView::droneScenePos(const QPointF &gridPos) const
{
    return QPointF((gridPos.x() - gridPos.y()) * TILE_W / 2.0,
                   (gridPos.x() + gridPos.y()) * TILE_H / 2.0);
}

void MapView::mouseMoveEvent(QMouseEvent *event) {
    if (m_panning) {
        const QPointF currentScene = mapToScene(event->pos());
        const QPointF delta = currentScene - m_lastPanScene;
        centerOn(mapToScene(viewport()->rect().center()) - delta);
        m_lastPanScene = mapToScene(event->pos());
        m_detailPopup->hidePopup();
        event->accept();
        return;
    }

    const QPoint grid = sceneToGrid(mapToScene(event->pos()));
    if (grid.x() >= 0 && grid.x() < m_gameState->mapWidth() &&
        grid.y() >= 0 && grid.y() < m_gameState->mapHeight()) {
        const LandPlot &plot = m_gameState->plot(grid.x(), grid.y());
        const QString crop = plot.crop == CropType::None ? landStateName(plot.state)
                                                         : cropName(plot.crop) + QStringLiteral("树");
        double timeLeft = 0.0;
        if (plot.crop != CropType::None && plot.state == LandState::Planted) {
            const CropDef &def = CropData::get(plot.crop);
            timeLeft = qMax(0.0, def.growthTime * (100.0 - plot.progress) / 100.0);
        }
        m_detailPopup->showAt(event->globalPosition().toPoint(), crop,
                              static_cast<int>(plot.progress), plot.water,
                              plot.fertility, timeLeft);
    } else {
        m_detailPopup->hidePopup();
    }
    QGraphicsView::mouseMoveEvent(event);
}

void MapView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton || event->button() == Qt::MiddleButton) {
        m_panning = true;
        m_lastPanPoint = event->pos();
        m_lastPanScene = mapToScene(event->pos());
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
    QGraphicsView::mousePressEvent(event);
}

void MapView::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_panning && (event->button() == Qt::LeftButton || event->button() == Qt::MiddleButton)) {
        m_panning = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void MapView::wheelEvent(QWheelEvent *event) {
    const QPointF before = mapToScene(event->position().toPoint());
    double factor = (event->angleDelta().y() > 0) ? 1.15 : 1.0 / 1.15;
    const double nextZoom = qBound(0.35, m_zoom * factor, 3.2);
    factor = nextZoom / m_zoom;
    m_zoom = nextZoom;
    scale(factor, factor);
    const QPointF after = mapToScene(event->position().toPoint());
    const QPointF delta = after - before;
    translate(delta.x(), delta.y());
    event->accept();
}

void MapView::leaveEvent(QEvent *event)
{
    m_detailPopup->hidePopup();
    QGraphicsView::leaveEvent(event);
}
