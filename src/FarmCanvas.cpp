#include "FarmCanvas.h"

#include "levels.h"

#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>
#include <QWheelEvent>

#include <cmath>

FarmCanvas::FarmCanvas(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(200, 200);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setObjectName("FarmCanvas");
    setMouseTracking(true);

    auto *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this] {
        phase += 0.055;
        actionFlash = std::max(0.0, actionFlash - 0.045);
        update();
    });
    timer->start(33);
}

void FarmCanvas::flashAction()
{
    actionFlash = 1.0;
}

void FarmCanvas::resetView()
{
    zoom = 1.0;
    panOffset = {0, 0};
    update();
}

double FarmCanvas::tileW() const
{
    if (grid.isEmpty()) return 80.0;
    const double base = std::min(108.0, (width() - 120.0) / std::max(1.0, (grid.width() + grid.height()) / 2.0 + 0.8));
    return base * zoom;
}

double FarmCanvas::tileH() const
{
    return tileW() * 0.55;
}

QPointF FarmCanvas::origin() const
{
    return QPointF(width() / 2.0 + panOffset.x(), height() * 0.40 + panOffset.y());
}

QPointF FarmCanvas::cellCenter(int x, int y) const
{
    const double tw = tileW();
    const double th = tileH();
    const QPointF o = origin();
    return QPointF(o.x() + (x - y) * tw * 0.5, o.y() + (x + y) * th * 0.5);
}

int FarmCanvas::cellAtPos(const QPointF &pos) const
{
    if (!cells || grid.isEmpty()) return -1;

    const double tw = tileW();
    const double th = tileH();
    const QPointF o = origin();
    const double dx = pos.x() - o.x();
    const double dy = pos.y() - o.y();
    const double fx = (dx / (tw * 0.5) + dy / (th * 0.5)) * 0.5;
    const double fy = (dy / (th * 0.5) - dx / (tw * 0.5)) * 0.5;
    const int gx = int(std::floor(fx + 0.5));
    const int gy = int(std::floor(fy + 0.5));

    if (gx < 0 || gy < 0 || gx >= grid.width() || gy >= grid.height()) return -1;
    return gy * grid.width() + gx;
}

void FarmCanvas::showCellTooltip(int cellIdx, const QPoint &globalPos)
{
    if (!cells || cellIdx < 0 || cellIdx >= cells->size()) {
        QToolTip::hideText();
        return;
    }

    const Cell &cell = cells->at(cellIdx);
    QString tip;
    const int x = cellIdx % grid.width();
    const int y = cellIdx / grid.width();

    tip += QString("位置: (%1, %2)\n").arg(x).arg(y);
    tip += cell.tilled ? "已开垦" : "未开垦";
    if (!cell.crop.isEmpty()) {
        const CropInfo *info = cropInfo(cell.crop);
        tip += QString("\n作物: %1 %2").arg(info ? info->icon : "?").arg(info ? info->name : cell.crop);
        tip += QString("\n成长: %1%").arg(int(cell.progress * 100));
        tip += QString("\n水分: %1%").arg(int(cell.water * 100));
        if (cell.fertilizer > 0) tip += "\n已施肥";
        if (cell.bug) tip += "\n有 Bug!";
    } else {
        tip += QString("\n水分: %1%").arg(int(cell.water * 100));
    }

    QToolTip::showText(globalPos, tip, this);
}

void FarmCanvas::wheelEvent(QWheelEvent *event)
{
    const double delta = event->angleDelta().y() > 0 ? 1.12 : 0.89;
    zoom = std::clamp(zoom * delta, 0.4, 3.5);
    update();
}

void FarmCanvas::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton || event->button() == Qt::MiddleButton) {
        panning = true;
        panStart = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
    QWidget::mousePressEvent(event);
}

void FarmCanvas::mouseMoveEvent(QMouseEvent *event)
{
    if (panning) {
        const QPoint delta = event->pos() - panStart;
        panOffset += QPointF(delta);
        panStart = event->pos();
        update();
    } else {
        const int idx = cellAtPos(event->position());
        if (idx != hoveredCell) {
            hoveredCell = idx;
            update();
        }
        if (idx >= 0) showCellTooltip(idx, event->globalPosition().toPoint());
        else QToolTip::hideText();
    }
    QWidget::mouseMoveEvent(event);
}

void FarmCanvas::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton || event->button() == Qt::MiddleButton) {
        panning = false;
        setCursor(Qt::ArrowCursor);
    }
    QWidget::mouseReleaseEvent(event);
}

void FarmCanvas::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QLinearGradient sky(rect().topLeft(), QPointF(rect().left(), rect().bottom()));
    sky.setColorAt(0.0, QColor("#cfe9ff"));
    sky.setColorAt(0.4, QColor("#f9f2d8"));
    sky.setColorAt(1.0, QColor("#d3e7b5"));
    p.fillRect(rect(), sky);

    QRadialGradient sun(QPointF(width() * 0.83, height() * 0.16), width() * 0.42);
    sun.setColorAt(0.0, QColor(255, 241, 183, 170));
    sun.setColorAt(0.45, QColor(255, 241, 183, 40));
    sun.setColorAt(1.0, QColor(255, 241, 183, 0));
    p.fillRect(rect(), sun);

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(180, 214, 134, 150));
    p.drawEllipse(QRectF(width() * 0.05, height() * 0.08, width() * 0.48, height() * 0.18));
    p.setBrush(QColor(137, 182, 89, 165));
    p.drawEllipse(QRectF(width() * 0.45, height() * 0.12, width() * 0.50, height() * 0.18));

    p.setPen(QPen(QColor(167, 133, 84, 58), 2.0));
    for (int row = 0; row < 7; ++row) {
        const double y = height() * 0.58 + row * 28.0;
        p.drawLine(QPointF(width() * 0.04, y), QPointF(width() * 0.96, y));
    }

    if (!cells || grid.isEmpty()) return;

    const double tw = tileW();
    const double th = tileH();
    const QPointF o = origin();

    for (int y = grid.height() - 1; y >= 0; --y) {
        for (int x = 0; x < grid.width(); ++x) {
            const int i = y * grid.width() + x;
            const QPointF c(o.x() + (x - y) * tw * 0.5, o.y() + (x + y) * th * 0.5);
            drawCell(p, c, tw, th, cells->at(i), i == selected, i == hoveredCell);
        }
    }

    const QPointF dronePos(
        o.x() + (drone.x() - drone.y()) * tw * 0.5,
        o.y() + (drone.x() + drone.y()) * th * 0.5 - th * 0.72 + std::sin(phase * 2.4) * 4.0 * zoom);
    drawDrone(p, dronePos, tw, th);
}

void FarmCanvas::drawCell(QPainter &p, const QPointF &c, double w, double h, const Cell &cell, bool active, bool hovered)
{
    const double sideH = 15 * zoom;

    QPolygonF top;
    top << QPointF(c.x(), c.y() - h / 2.0) << QPointF(c.x() + w / 2.0, c.y())
        << QPointF(c.x(), c.y() + h / 2.0) << QPointF(c.x() - w / 2.0, c.y());

    QPolygonF sideR;
    sideR << QPointF(c.x() + w / 2.0, c.y()) << QPointF(c.x(), c.y() + h / 2.0)
          << QPointF(c.x(), c.y() + h / 2.0 + sideH) << QPointF(c.x() + w / 2.0, c.y() + sideH);

    QPolygonF sideL;
    sideL << QPointF(c.x() - w / 2.0, c.y()) << QPointF(c.x(), c.y() + h / 2.0)
          << QPointF(c.x(), c.y() + h / 2.0 + sideH) << QPointF(c.x() - w / 2.0, c.y() + sideH);

    QColor base = cell.tilled ? QColor("#9c6f46") : QColor("#b7d889");
    if (!cell.crop.isEmpty()) {
        base = QColor("#86bb63");
        if (cell.progress > 0.66) base = QColor("#d4b55a");
        if (cell.bug) base = QColor("#cf7d66");
    }

    p.setPen(Qt::NoPen);
    p.setBrush(base.darker(112));
    p.drawPolygon(sideR);
    p.setBrush(base.darker(128));
    p.drawPolygon(sideL);

    QLinearGradient tileGrad(QPointF(c.x(), c.y() - h * 0.5), QPointF(c.x(), c.y() + h * 0.5));
    tileGrad.setColorAt(0, base.lighter(114));
    tileGrad.setColorAt(1, base);

    QColor borderColor(99, 128, 73, 58);
    double borderW = 1.2;
    if (active) {
        borderColor = QColor("#f0a545");
        borderW = 2.6;
    } else if (hovered) {
        borderColor = QColor("#6a9c48");
        borderW = 2.0;
    }

    p.setPen(QPen(borderColor, borderW));
    p.setBrush(tileGrad);
    p.drawPolygon(top);

    if (active) {
        QColor pulse("#f6b95e");
        pulse.setAlphaF(0.16 + 0.10 * ((std::sin(phase * 3.0) + 1.0) * 0.5));
        p.setPen(QPen(pulse, 4 * zoom));
        p.setBrush(Qt::NoBrush);
        p.drawPolygon(top);
        if (actionFlash > 0.01) {
            QColor flash("#fff0a2");
            flash.setAlphaF(actionFlash * 0.42);
            p.setPen(QPen(flash, 3 * zoom));
            p.drawPolygon(top);
        }
    }

    if (!cell.tilled && cell.crop.isEmpty()) {
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(112, 174, 86, 85));
        const double gs = w * 0.042;
        for (int i = 0; i < 6; ++i) {
            const double gx = c.x() + (i - 2.5) * w * 0.11 + std::sin(phase + i) * 1.8;
            const double gy = c.y() + (i % 2 - 0.5) * h * 0.2;
            p.drawEllipse(QPointF(gx, gy), gs, gs * 2.4);
        }
    }

    if (cell.tilled && cell.crop.isEmpty()) {
        p.setPen(QPen(QColor(132, 89, 46, 75), 1.4 * zoom));
        for (int i = 0; i < 3; ++i) {
            const double offset = (i - 1) * h * 0.22;
            p.drawLine(QPointF(c.x() - w * 0.28, c.y() + offset), QPointF(c.x() + w * 0.28, c.y() + offset));
        }
    }

    if (cell.water > 0.05) {
        QColor water("#71c8ef");
        water.setAlphaF(std::min(0.28, cell.water * 0.24));
        p.setBrush(water);
        p.setPen(Qt::NoPen);
        p.drawPolygon(top);

        const double waveY = std::sin(phase * 2.4) * 2.0 * zoom;
        p.setPen(QPen(QColor(103, 185, 227, int(cell.water * 125)), 1.3 * zoom));
        p.drawLine(QPointF(c.x() - w * 0.2, c.y() + waveY), QPointF(c.x() + w * 0.2, c.y() + waveY));
    }

    if (!cell.crop.isEmpty()) {
        const CropInfo *info = cropInfo(cell.crop);
        const double plantH = h * 0.34 * std::max(0.18, cell.progress);
        const double stemX = c.x();
        const double stemBase = c.y() + h * 0.08;

        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#5a9642"));
        p.drawRoundedRect(QRectF(stemX - 1.6 * zoom, stemBase - plantH, 3.2 * zoom, plantH), 1.4, 1.4);

        if (cell.progress > 0.24) {
            p.setBrush(QColor("#7bb65f"));
            const double leafW = w * 0.075 * std::min(1.0, cell.progress * 1.4);
            p.drawEllipse(QPointF(stemX - leafW, stemBase - plantH * 0.62), leafW, leafW * 0.58);
            p.drawEllipse(QPointF(stemX + leafW, stemBase - plantH * 0.42), leafW, leafW * 0.58);
        }

        if (cell.progress >= 1.0) {
            p.setBrush(QColor("#f2c85a"));
            const double fruitR = w * 0.065;
            p.drawEllipse(QPointF(stemX, stemBase - plantH - fruitR * 0.9), fruitR, fruitR);
        }

        QRectF bar(c.x() - w * 0.28, c.y() + h * 0.2, w * 0.56, 5.5 * zoom);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 251, 240, 155));
        p.drawRoundedRect(bar, 2.5, 2.5);
        p.setBrush(cell.bug ? QColor("#d87b64") : QColor("#7db25d"));
        p.drawRoundedRect(QRectF(bar.left(), bar.top(), bar.width() * cell.progress, bar.height()), 2.5, 2.5);

        if (info) {
            p.setPen(QColor("#fff8ea"));
            QFont f = p.font();
            f.setPointSizeF(std::max(8.0, w * 0.09));
            p.setFont(f);
            p.drawText(QRectF(c.x() + w * 0.12, c.y() - h * 0.42, w * 0.34, h * 0.3), Qt::AlignCenter, info->icon);
        }
    }

    if (cell.fertilizer > 0) {
        p.setBrush(QColor(250, 208, 93, 160));
        p.setPen(Qt::NoPen);
        const double r = 3.8 * zoom + std::sin(phase * 4.0) * 0.8 * zoom;
        p.drawEllipse(QPointF(c.x() + w * 0.2, c.y() - h * 0.14), r, r);
    }

    if (cell.bug) {
        p.setPen(QPen(QColor("#b94e41"), 1.5 * zoom));
        const double bx = c.x() - w * 0.22;
        const double by = c.y() - h * 0.28;
        const double bs = 5.5 * zoom;
        p.drawLine(QPointF(bx, by), QPointF(bx + bs, by + bs));
        p.drawLine(QPointF(bx + bs, by), QPointF(bx, by + bs));
    }
}

void FarmCanvas::drawDrone(QPainter &p, const QPointF &c, double w, double h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);
    const double s = zoom;

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 38));
    p.drawEllipse(QPointF(c.x(), c.y() + 30 * s), 18 * s, 6 * s);

    QLinearGradient bodyGrad(QPointF(c.x(), c.y() - 10 * s), QPointF(c.x(), c.y() + 10 * s));
    bodyGrad.setColorAt(0, QColor("#fff7e4"));
    bodyGrad.setColorAt(1, QColor("#e3d8b8"));
    p.setBrush(bodyGrad);
    p.drawRoundedRect(QRectF(c.x() - 18 * s, c.y() - 9 * s, 36 * s, 18 * s), 9 * s, 9 * s);

    p.setBrush(QColor("#88b566"));
    p.drawEllipse(QRectF(c.x() - 7 * s, c.y() - 5 * s, 14 * s, 10 * s));
    p.setBrush(QColor(255, 255, 255, 210));
    p.drawEllipse(QRectF(c.x() - 2 * s, c.y() - 3 * s, 4 * s, 3 * s));

    const double ledPhase = std::fmod(phase * 3.0, 6.28);
    const int ledAlpha = int(135 + 95 * std::sin(ledPhase));
    p.setBrush(QColor(237, 175, 64, ledAlpha));
    p.drawEllipse(QPointF(c.x() + 12 * s, c.y() - 4 * s), 2.5 * s, 2.5 * s);

    p.setPen(QPen(QColor("#7c9461"), 2.5 * s));
    p.drawLine(QPointF(c.x() - 16 * s, c.y()), QPointF(c.x() - 32 * s, c.y() - 10 * s));
    p.drawLine(QPointF(c.x() + 16 * s, c.y()), QPointF(c.x() + 32 * s, c.y() - 10 * s));

    const double propAngle = phase * 18.0;
    const double propLen = 14 * s;
    for (int arm = 0; arm < 2; ++arm) {
        const double ax = (arm == 0) ? c.x() - 32 * s : c.x() + 32 * s;
        const double ay = c.y() - 10 * s;
        const double a1 = propAngle + arm * 1.57;
        const double a2 = a1 + 3.14;
        p.setPen(QPen(QColor(126, 154, 159, 185), 2.0 * s, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(ax + std::cos(a1) * propLen, ay + std::sin(a1) * propLen * 0.3),
                   QPointF(ax + std::cos(a2) * propLen, ay + std::sin(a2) * propLen * 0.3));
    }
}
