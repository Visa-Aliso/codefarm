#ifndef PARTICLEEFFECT_H
#define PARTICLEEFFECT_H

#include <QObject>
#include <QVector>
#include <QPointF>
#include <QGraphicsScene>

struct Particle {
    QGraphicsEllipseItem *item;
    QPointF velocity;
    double lifetime;
    double age;
};

class ParticleEffect : public QObject {
    Q_OBJECT
public:
    explicit ParticleEffect(QGraphicsScene *scene, QObject *parent = nullptr);

    void emitBurst(QPointF pos, const QColor &color, int count, double spread);
    void emitCone(QPointF pos, const QColor &color, int count, double angle, double spread);
    void update(double dt);

private:
    QGraphicsScene *m_scene;
    QVector<Particle> m_particles;
};

#endif // PARTICLEEFFECT_H
