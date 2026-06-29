#include "gameengine.h"
#include "farmmap.h"
#include "drone.h"
#include "goalsystem.h"
#include "python/pythonexecutor.h"
#include "levels/levelmanager.h"
#include "levels/leveldata.h"

GameEngine::GameEngine(QObject *parent)
    : QObject(parent)
    , tickTimer_(new QTimer(this))
    , map_(new FarmMap(this))
    , drone_(new Drone(this))
    , goals_(new GoalSystem(this))
    , executor_(new PythonExecutor(this))
{
    tickTimer_->setTimerType(Qt::PreciseTimer);
    connect(tickTimer_, &QTimer::timeout, this, &GameEngine::onTick);
    executor_->setContext(map_, drone_, goals_);

    connect(drone_, &Drone::positionChanged, this, &GameEngine::dronePositionChanged);
    connect(drone_, &Drone::energyChanged, this, &GameEngine::energyChanged);
    connect(goals_, &GoalSystem::goalCompleted, this, [this](int index) {
        emit goalCompleted(index);
        emit goalsChanged();
        emit logMessage(QStringLiteral("目标 %1 已完成").arg(index + 1), QStringLiteral("#27AE60"));
    });
    connect(executor_, &PythonExecutor::lineExecuting, this, &GameEngine::lineExecuting);
    connect(executor_, &PythonExecutor::printOutput, this, [this](const QString &text) {
        emit logMessage(text, QStringLiteral("#6B7F6C"));
    });
    connect(executor_, &PythonExecutor::apiCalled, this, [this](const QString &funcName, bool result) {
        const QString color = result ? QStringLiteral("#3B8B4A") : QStringLiteral("#D94F4F");
        emit logMessage(QStringLiteral("%1() -> %2").arg(funcName, result ? QStringLiteral("True")
                                                                           : QStringLiteral("False")),
                        color);
    });
    connect(executor_, &PythonExecutor::errorOccurred, this, [this](const QString &msg, int line) {
        executor_->stopScript();
        stopElapsedTimer();
        tickTimer_->stop();
        state_ = Error;
        emit stateChanged();
        emit errorOccurred(msg, line);
        emit logMessage(QStringLiteral("脚本错误: %1").arg(msg), QStringLiteral("#D94F4F"));
    });
}

GameEngine::~GameEngine() = default;

int GameEngine::timeElapsed() const {
    qint64 elapsed = accumulatedElapsedMs_;
    if (elapsedTimerRunning_ && gameTimer_.isValid()) {
        elapsed += gameTimer_.elapsed();
    }
    return static_cast<int>(elapsed / 1000);
}

float GameEngine::energy() const { return drone_->energy(); }
float GameEngine::maxEnergy() const { return drone_->maxEnergy(); }
int GameEngine::droneX() const { return drone_->x(); }
int GameEngine::droneY() const { return drone_->y(); }

void GameEngine::startElapsedTimer() {
    if (elapsedTimerRunning_) {
        return;
    }
    gameTimer_.start();
    elapsedTimerRunning_ = true;
}

void GameEngine::stopElapsedTimer() {
    if (!elapsedTimerRunning_ || !gameTimer_.isValid()) {
        return;
    }
    accumulatedElapsedMs_ += gameTimer_.elapsed();
    elapsedTimerRunning_ = false;
}

void GameEngine::loadLevel(int levelId) {
    if (!levelManager_) return;

    const LevelConfig cfg = levelManager_->getLevelConfig(levelId);
    if (cfg.levelId == 0) {
        emit errorOccurred(QStringLiteral("关卡不存在"), 0);
        return;
    }

    tickTimer_->stop();
    executor_->stopScript();
    stopElapsedTimer();
    accumulatedElapsedMs_ = 0;
    state_ = Idle;
    tickCount_ = 0;
    currentLevelId_ = levelId;
    currentLevelName_ = cfg.name;
    maxTimeSec_ = cfg.maxTimeSec;
    currentStartX_ = cfg.droneStartX;
    currentStartY_ = cfg.droneStartY;
    currentBugProbability_ = cfg.bugProbability;
    currentDroneMaxEnergy_ = cfg.droneMaxEnergy;

    map_->init(cfg.gridW, cfg.gridH, cfg.presetCells);
    drone_->reset(cfg.droneStartX, cfg.droneStartY, cfg.droneMaxEnergy);
    goals_->init(cfg.goals);
    executor_->setAllowedFunctions(cfg.allowedFunctions);
    executor_->setAllowedSyntax(cfg.allowedSyntax);
    executor_->setAllowedCrops(cfg.allowedCrops);
    executor_->loadScript(cfg.tutorialCode);
    executor_->resetState();

    tutorialCode_ = cfg.tutorialCode;
    emit stateChanged();
    emit currentLevelChanged();
    emit tutorialCodeChanged();
    emit tickExecuted(0);
    emit timeChanged();
    emit energyChanged();
    emit dronePositionChanged(drone_->x(), drone_->y());
    emit goalsChanged();
    emit logMessage(QStringLiteral("已载入关卡：%1").arg(currentLevelName_), QStringLiteral("#6B7F6C"));
}

QVariantList GameEngine::goals() const {
    return goals_->goalsModel();
}

void GameEngine::loadScript(const QString &code) {
    executor_->loadScript(code);
}

void GameEngine::run() {
    if (state_ == Idle) {
        if (!executor_->startScript()) {
            return;
        }
    }
    startElapsedTimer();
    state_ = Running;
    tickTimer_->start(static_cast<int>(500.0f / speedMultiplier_));
    emit stateChanged();
}

void GameEngine::pause() {
    stopElapsedTimer();
    state_ = Paused;
    tickTimer_->stop();
    emit stateChanged();
    emit timeChanged();
}

void GameEngine::stepOnce() {
    if (state_ == Idle) {
        if (!executor_->startScript()) {
            return;
        }
    }
    startElapsedTimer();
    state_ = Paused;
    tickTimer_->stop();
    onTick();
    stopElapsedTimer();
    emit stateChanged();
    emit timeChanged();
}

void GameEngine::reset() {
    tickTimer_->stop();
    executor_->stopScript();
    stopElapsedTimer();
    accumulatedElapsedMs_ = 0;
    state_ = Idle;
    tickCount_ = 0;
    executor_->resetState();
    map_->resetToPreset();
    drone_->reset(currentStartX_, currentStartY_, currentDroneMaxEnergy_);
    goals_->reset();
    emit stateChanged();
    emit tickExecuted(0);
    emit timeChanged();
    emit energyChanged();
    emit goalsChanged();
    emit dronePositionChanged(drone_->x(), drone_->y());
    emit logMessage(QStringLiteral("关卡已重置"), QStringLiteral("#6B7F6C"));
}

void GameEngine::setSpeed(float multiplier) {
    speedMultiplier_ = qBound(0.1f, multiplier, 5.0f);
    if (state_ == Running) {
        tickTimer_->setInterval(static_cast<int>(500.0f / speedMultiplier_));
    }
}

void GameEngine::giveUp() {
    tickTimer_->stop();
    executor_->stopScript();
    stopElapsedTimer();
    state_ = Idle;
    emit stateChanged();
    emit levelFailed("player_quit");
}

void GameEngine::onTick() {
    tickCount_++;
    drone_->regenEnergy();
    emit energyChanged();

    bool ok = executor_->executeTick();
    if (!ok) {
        return;
    }

    map_->tickUpdate(currentBugProbability_);
    const int elapsedSec = timeElapsed();
    goals_->checkAll(elapsedSec);
    executor_->finishTick();

    emit tickExecuted(tickCount_);
    emit timeChanged();
    emit goalsChanged();

    if (goals_->allRequiredCompleted()) {
        stopElapsedTimer();
        state_ = Idle;
        tickTimer_->stop();
        goals_->finalizeTimeGoals(elapsedSec);
        emit stateChanged();
        emit goalsChanged();
        emit levelCleared(goals_->starsEarned());
        return;
    }

    if (executor_->isScriptCompleted()) {
        stopElapsedTimer();
        state_ = Idle;
        tickTimer_->stop();
        emit stateChanged();
        emit logMessage(QStringLiteral("脚本已执行完毕，但目标尚未完成"), QStringLiteral("#D94F4F"));
        emit scriptCompletedWithoutGoals();
        return;
    }

    if (maxTimeSec_ > 0 && elapsedSec >= maxTimeSec_) {
        executor_->stopScript();
        stopElapsedTimer();
        state_ = Idle;
        tickTimer_->stop();
        emit stateChanged();
        emit levelFailed(QStringLiteral("timeout"));
    }
}
