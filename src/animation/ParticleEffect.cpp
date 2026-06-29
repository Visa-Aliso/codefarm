#include "ParticleEffect.h"
#include <QRandomGenerator>
#include <QGraphicsEllipseItem>
#include <cmath>

ParticleEffect::ParticleEffect(QGraphicsScene *scene, QObject *parent)
    : QObject(parent)
    , m_scene(scene)
{
}

void ParticleEffect::emitBurst(QPointF pos, const QColor &color, int count, double spread) {
    for (int i = 0; i < count; ++i) {
        double angle = QRandomGenerator::global()->bounded(2.0 * M_PI);
        double speed = QRandomGenerator::global()->bounded(spread);
        QPointF vel(std::cos(angle) * speed, std::sin(angle) * speed);

        auto *item = m_scene->addEllipse(-2, -2, 4, 4, QPen(Qt::NoPen), color);
        item->setPos(pos);
        item->setZValue(2000);

        m_particles.append({item, vel, 0.6 + QRandomGenerator::global()->bounded(0.4), 0.0});
    }
}

void ParticleEffect::emitCone(QPointF pos, const QColor &color, int count,
                               double angle, double spread) {
    for (int i = 0; i < count; ++i) {
        double a = angle + (QRandomGenerator::global()->bounded(spread) - spread / 2.0);
        double speed = 30 + QRandomGenerator::global()->bounded(40);
        QPointF vel(std::cos(a) * speed, std::sin(a) * speed);

        auto *item = m_scene->addEllipse(-2.5, -2.5, 5, 5, QPen(Qt::NoPen), color);
        item->setPos(pos);
        item->setZValue(2000);

        m_particles.append({item, vel, 0.5 + QRandomGenerator::global()->bounded(0.3), 0.0});
    }
}

void ParticleEffect::update(double dt) {
    for (int i = m_particles.size() - 1; i >= 0; --i) {
        Particle &p = m_particles[i];
        p.age += dt;
        if (p.age >= p.lifetime) {
            m_scene->removeItem(p.item);
            delete p.item;
            m_particles.removeAt(i);
        } else {
            p.item->moveBy(p.velocity.x() * dt, p.velocity.y() * dt);
            p.velocity *= 0.95; // drag
            double alpha = 1.0 - p.age / p.lifetime;
            p.item->setOpacity(alpha);
            p.item->setScale(1.0 - p.age / p.lifetime);
        }
    }
}
