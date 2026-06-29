#include "FarmBackground.h"

#include <QPainter>
#include <QPainterPath>

#include <cmath>

FarmBackground::FarmBackground(QWidget *parent) : QWidget(parent)
{
    setFixedHeight(128);
    connect(&timer, &QTimer::timeout, this, [this] {
        phase += 0.02;
        update();
    });
    timer.start(50);
}

void FarmBackground::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int w = width();
    const int h = height();

    QLinearGradient sky(0, 0, 0, h);
    sky.setColorAt(0.0, QColor("#d8edff"));
    sky.setColorAt(0.55, QColor("#eef6dc"));
    sky.setColorAt(1.0, QColor("#d7e9bb"));
    p.fillRect(rect(), sky);

    QRadialGradient sun(QPointF(w * 0.82, h * 0.18), h * 0.72);
    sun.setColorAt(0.0, QColor(255, 241, 177, 150));
    sun.setColorAt(0.55, QColor(255, 241, 177, 45));
    sun.setColorAt(1.0, QColor(255, 241, 177, 0));
    p.fillRect(rect(), sun);

    p.setPen(Qt::NoPen);
    for (int i = 0; i < 4; ++i) {
        const double cx = std::fmod(w * 0.18 * (i + 1) + phase * 12.0 * (i % 2 ? 1.0 : -1.0) + w, double(w));
        const double cy = 20.0 + i * 6.0 + 4.0 * std::sin(phase * 0.6 + i);
        p.setBrush(QColor(255, 255, 255, 110));
        p.drawEllipse(QPointF(cx, cy), 28, 12);
        p.drawEllipse(QPointF(cx + 18, cy + 2), 22, 10);
        p.drawEllipse(QPointF(cx - 16, cy + 3), 20, 9);
    }

    QPainterPath hillBack;
    hillBack.moveTo(0, h);
    for (int x = 0; x <= w; x += 4) {
        const double y = h - 56 + 14 * std::sin(x * 0.007 + phase * 0.22) + 7 * std::sin(x * 0.017 + 1.5);
        hillBack.lineTo(x, y);
    }
    hillBack.lineTo(w, h);
    hillBack.closeSubpath();
    p.fillPath(hillBack, QColor("#bfd89a"));

    QPainterPath hillMid;
    hillMid.moveTo(0, h);
    for (int x = 0; x <= w; x += 4) {
        const double y = h - 34 + 12 * std::sin(x * 0.011 + phase * 0.35 + 1.8)
                       + 5 * std::sin(x * 0.019 + 0.7);
        hillMid.lineTo(x, y);
    }
    hillMid.lineTo(w, h);
    hillMid.closeSubpath();
    p.fillPath(hillMid, QColor("#9ec471"));

    QPainterPath ground;
    ground.moveTo(0, h);
    for (int x = 0; x <= w; x += 4) {
        const double y = h - 12 + 4 * std::sin(x * 0.018 + phase * 0.65 + 3.8);
        ground.lineTo(x, y);
    }
    ground.lineTo(w, h);
    ground.closeSubpath();
    p.fillPath(ground, QColor("#7aa252"));

    p.setPen(QPen(QColor(140, 105, 62, 60), 2));
    for (int row = 0; row < 6; ++row) {
        const double y = h - 9 - row * 8;
        p.drawLine(QPointF(0, y), QPointF(w, y - 4));
    }

    p.setPen(Qt::NoPen);
    for (int i = 0; i < 10; ++i) {
        const double cx = w * (0.08 + i * 0.09) + 10 * std::sin(phase * 0.4 + i);
        const double baseY = h - 22 + 4 * std::sin(cx * 0.018 + phase * 0.5);
        const double cropH = 8 + 5 * std::sin(i * 0.9 + phase * 0.8);
        p.setBrush(QColor(74, 132, 56, 130));
        p.drawEllipse(QPointF(cx, baseY - cropH), 3, cropH);
        p.setBrush(QColor(219, 188, 95, 110));
        p.drawEllipse(QPointF(cx + 1.5, baseY - cropH - 5), 3.5, 3.5);
    }

    p.setFont(QFont("JetBrains Mono", 9));
    static const char *snippets[] = {"()", "[]", "{}", "for", "if"};
    for (int i = 0; i < 5; ++i) {
        const double px = std::fmod(i * 205.0 + phase * 18.0 + w, double(w));
        const double py = h - 52 - 22 * std::sin(phase * 0.55 + i * 1.4);
        const int alpha = int(38 + 18 * std::sin(phase + i * 1.1));
        p.setPen(QColor(103, 145, 54, alpha));
        p.drawText(QPointF(px, py), snippets[i % 5]);
    }
}
