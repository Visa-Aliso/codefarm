#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QVariantList>

class FarmMap;
class Drone;
class GoalSystem;
class PythonExecutor;
class LevelManager;

class GameEngine : public QObject {
    Q_OBJECT

    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(int tickCount READ tickCount NOTIFY tickExecuted)
    Q_PROPERTY(int timeElapsed READ timeElapsed NOTIFY timeChanged)
    Q_PROPERTY(int droneX READ droneX NOTIFY dronePositionChanged)
    Q_PROPERTY(int droneY READ droneY NOTIFY dronePositionChanged)
    Q_PROPERTY(QVariantList goals READ goals NOTIFY goalsChanged)
    Q_PROPERTY(QString tutorialCode READ tutorialCode NOTIFY tutorialCodeChanged)
    Q_PROPERTY(int currentLevelId READ currentLevelId NOTIFY currentLevelChanged)
    Q_PROPERTY(QString currentLevelName READ currentLevelName NOTIFY currentLevelChanged)
    Q_PROPERTY(int maxTimeSec READ maxTimeSec NOTIFY currentLevelChanged)

public:
    enum State { Idle, Running, Paused, Error };
    Q_ENUM(State)

    explicit GameEngine(QObject *parent = nullptr);
    ~GameEngine();

    State state() const { return state_; }
    int tickCount() const { return tickCount_; }
    int timeElapsed() const;
    int droneX() const;
    int droneY() const;
    QVariantList goals() const;
    QString tutorialCode() const { return tutorialCode_; }
    int currentLevelId() const { return currentLevelId_; }
    QString currentLevelName() const { return currentLevelName_; }
    int maxTimeSec() const { return maxTimeSec_; }

    FarmMap* farmMap() const { return map_; }
    void setLevelManager(LevelManager *lm) { levelManager_ = lm; }

    Q_INVOKABLE void loadLevel(int levelId);
    Q_INVOKABLE void loadScript(const QString &code);
    Q_INVOKABLE void run();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void stepOnce();
    Q_INVOKABLE void reset();
    Q_INVOKABLE void setSpeed(float multiplier);
    Q_INVOKABLE void giveUp();

signals:
    void stateChanged();
    void tickExecuted(int tickNum);
    void timeChanged();
    void dronePositionChanged(int x, int y);
    void goalCompleted(int goalIndex);
    void goalsChanged();
    void levelCleared(int stars);
    void levelFailed(const QString &reason);
    void scriptCompletedWithoutGoals();
    void logMessage(const QString &text, const QString &color);
    void errorOccurred(const QString &msg, int line);
    void lineExecuting(int line);
    void tutorialCodeChanged();
    void currentLevelChanged();

private slots:
    void onTick();
    void onUiTimerTick();

private:
    void startElapsedTimer();
    void stopElapsedTimer();

    QTimer *tickTimer_ = nullptr;
    QTimer *uiTimer_ = nullptr;
    QElapsedTimer gameTimer_;
    FarmMap *map_ = nullptr;
    Drone *drone_ = nullptr;
    PythonExecutor *executor_ = nullptr;
    GoalSystem *goals_ = nullptr;
    State state_ = Idle;
    int tickCount_ = 0;
    int currentLevelId_ = -1;
    QString currentLevelName_;
    float speedMultiplier_ = 1.0f;
    QString tutorialCode_;
    int maxTimeSec_ = 0;
    int currentStartX_ = 0;
    int currentStartY_ = 0;
    float currentBugProbability_ = 0.005f;
    qint64 accumulatedElapsedMs_ = 0;
    bool elapsedTimerRunning_ = false;
    LevelManager *levelManager_ = nullptr;
};

#endif // GAMEENGINE_H
