#include "AnimationManager.h"
#include "core/GameState.h"
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QRandomGenerator>

AnimationManager::AnimationManager(GameState *state, QGraphicsScene *scene,
                                     QObject *parent)
    : QObject(parent)
    , m_gameState(state)
    , m_scene(scene)
{
    connect(&m_timer, &QTimer::timeout, this, &AnimationManager::tick);
}

void AnimationManager::start() {
    m_elapsed.start();
    m_timer.start(16); // ~60fps
}

void AnimationManager::stop() {
    m_timer.stop();
}

void AnimationManager::tick() {
    updateCropSway();
    updateParticles();
}

void AnimationManager::updateCropSway() {
    // Gentle sway for planted tiles via Z-rotation
    // Implemented in Phase 8
}

void AnimationManager::updateParticles() {
    // Particle lifetime management
    // Implemented in Phase 8
}

void AnimationManager::playHarvestSparkle(QPointF worldPos) {
    // Create small star particles that scale and fade
    for (int i = 0; i < 6; ++i) {
        auto *star = m_scene->addEllipse(-3, -3, 6, 6,
                                          QPen(Qt::NoPen),
                                          QColor("#E8C86A"));
        star->setPos(worldPos + QPointF(
            QRandomGenerator::global()->bounded(-20, 20),
            QRandomGenerator::global()->bounded(-20, 20)));
        star->setZValue(2000);
    }
}

void AnimationManager::playWaterSpray(QPointF worldPos) {
    for (int i = 0; i < 8; ++i) {
        auto *drop = m_scene->addEllipse(-2, -2, 4, 4,
                                          QPen(Qt::NoPen),
                                          QColor("#7EC8E3"));
        drop->setPos(worldPos + QPointF(
            QRandomGenerator::global()->bounded(-15, 15),
            QRandomGenerator::global()->bounded(-25, -5)));
        drop->setZValue(2000);
    }
}

void AnimationManager::playBugSpray(QPointF worldPos) {
    for (int i = 0; i < 10; ++i) {
        auto *mist = m_scene->addEllipse(-3, -3, 6, 6,
                                          QPen(Qt::NoPen),
                                          QColor("#8FBC8F"));
        mist->setPos(worldPos + QPointF(
            QRandomGenerator::global()->bounded(-20, 20),
            QRandomGenerator::global()->bounded(-20, 20)));
        mist->setZValue(2000);
    }
}
