#pragma once

#include "types.h"
#include <QWidget>
#include <QTimer>
#include <QVector>
#include <QPoint>
#include <QPointF>
#include <QToolTip>

class FarmCanvas final : public QWidget {
    Q_OBJECT
public:
    QVector<Cell> *cells = nullptr;
    QSize grid;
    QPoint drone;
    int selected = -1;

    explicit FarmCanvas(QWidget *parent = nullptr);
    void flashAction();
    void resetView();

protected:
    void paintEvent(QPaintEvent *) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    double phase = 0;
    double actionFlash = 0;
    double zoom = 1.0;
    QPointF panOffset{0, 0};
    bool panning = false;
    QPoint panStart;
    int hoveredCell = -1;

    QPointF cellCenter(int x, int y) const;
    int cellAtPos(const QPointF &pos) const;
    double tileW() const;
    double tileH() const;
    QPointF origin() const;

    void drawCell(QPainter &p, const QPointF &c, double w, double h, const Cell &cell, bool active, bool hovered);
    void drawDrone(QPainter &p, const QPointF &c, double w, double h);
    void showCellTooltip(int cellIdx, const QPoint &globalPos);
};
