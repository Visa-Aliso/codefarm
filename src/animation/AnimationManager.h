#ifndef ANIMATIONMANAGER_H
#define ANIMATIONMANAGER_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>

class GameState;
class QGraphicsScene;

class AnimationManager : public QObject {
    Q_OBJECT
public:
    explicit AnimationManager(GameState *state, QGraphicsScene *scene,
                               QObject *parent = nullptr);

    void start();
    void stop();

    // Trigger effects
    void playHarvestSparkle(QPointF worldPos);
    void playWaterSpray(QPointF worldPos);
    void playBugSpray(QPointF worldPos);

private:
    void tick();
    void updateCropSway();
    void updateParticles();

    GameState *m_gameState;
    QGraphicsScene *m_scene;
    QTimer m_timer;
    QElapsedTimer m_elapsed;
};

#endif // ANIMATIONMANAGER_H
