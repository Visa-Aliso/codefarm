#include "appviewmodel.h"

#include "core/farmmap.h"
#include "core/gameengine.h"
#include "levels/levelmanager.h"
#include "save/savemanager.h"

#include <QSettings>
#include <QTimer>

AppViewModel::AppViewModel(GameEngine *engine,
                           LevelManager *levelManager,
                           SaveManager *saveManager,
                           QObject *parent)
    : QObject(parent)
    , engine_(engine)
    , levelManager_(levelManager)
    , saveManager_(saveManager)
{
    loadPreferences();

    saveTimer_ = new QTimer(this);
    saveTimer_->setSingleShot(true);
    saveTimer_->setInterval(500);
    connect(saveTimer_, &QTimer::timeout, this, [this]() {
        if (saveManager_ && activeLevelId_ > 0) {
            saveManager_->saveScript(activeLevelId_, scriptText_);
        }
    });

    connect(levelManager_, &LevelManager::progressChanged, this, &AppViewModel::levelsChanged);
    connect(levelManager_, &LevelManager::levelUnlocked, this, &AppViewModel::levelsChanged);
    connect(engine_, &GameEngine::currentLevelChanged, this, [this]() {
        activeLevelId_ = engine_->currentLevelId();
        setConsoleLine(QStringLiteral("系统就绪，等待任务。"));
        emit activeLevelChanged();
        emit activeGoalsChanged();
        emit runtimeChanged();
        emit farmMapChanged();
        refreshScriptFromStore();
    });
    connect(engine_, &GameEngine::goalsChanged, this, &AppViewModel::activeGoalsChanged);
    connect(engine_, &GameEngine::stateChanged, this, &AppViewModel::runtimeChanged);
    connect(engine_, &GameEngine::tickExecuted, this, [this](int) { emit runtimeChanged(); });
    connect(engine_, &GameEngine::timeChanged, this, &AppViewModel::runtimeChanged);
    connect(engine_, &GameEngine::dronePositionChanged, this, [this](int, int) { emit runtimeChanged(); });
    connect(engine_, &GameEngine::logMessage, this, [this](const QString &text, const QString &) {
        setConsoleLine(text);
    });
    connect(engine_, &GameEngine::lineExecuting, this, [this](int line) {
        if (executingLine_ == line) {
            return;
        }
        executingLine_ = line;
        emit executingLineChanged();
    });
    connect(engine_, &GameEngine::stateChanged, this, [this]() {
        if (engine_->state() != GameEngine::Running && executingLine_ != -1) {
            executingLine_ = -1;
            emit executingLineChanged();
        }
    });
    connect(engine_, &GameEngine::levelCleared, this, &AppViewModel::levelCleared);
    connect(engine_, &GameEngine::levelFailed, this, &AppViewModel::levelFailed);
    connect(engine_, &GameEngine::scriptCompletedWithoutGoals, this, [this]() {
        setConsoleLine(QStringLiteral("脚本已结束，目标未完成。可继续编辑后再次运行，或重置关卡。"));
        emit scriptCompletedWithoutGoals();
    });
    connect(engine_, &GameEngine::errorOccurred, this, &AppViewModel::errorOccurred);
}

QVariantList AppViewModel::levels() const {
    QVariantList out;
    if (!levelManager_) {
        return out;
    }
    for (int id = 1; id <= levelManager_->levelCount(); ++id) {
        QVariantMap item = levelManager_->getLevel(id);
        item.insert(QStringLiteral("unlocked"), levelManager_->isUnlocked(id));
        item.insert(QStringLiteral("stars"), levelManager_->getStars(id));
        item.insert(QStringLiteral("bestTime"), levelManager_->getBestTime(id));
        item.insert(QStringLiteral("clearCount"), levelManager_->getClearCount(id));
        out.append(item);
    }
    return out;
}

QVariantMap AppViewModel::activeLevel() const {
    return level(activeLevelId_ > 0 ? activeLevelId_ : engine_->currentLevelId());
}

QVariantList AppViewModel::activeGoals() const {
    return engine_ ? engine_->goals() : QVariantList {};
}

QVariantList AppViewModel::allowedApis() const {
    return activeLevel().value(QStringLiteral("allowedFunctions")).toList();
}

QObject *AppViewModel::farmMap() const {
    return engine_ ? engine_->farmMap() : nullptr;
}

int AppViewModel::state() const {
    return engine_ ? static_cast<int>(engine_->state()) : 0;
}

int AppViewModel::tickCount() const { return engine_ ? engine_->tickCount() : 0; }
int AppViewModel::timeElapsed() const { return engine_ ? engine_->timeElapsed() : 0; }
int AppViewModel::droneX() const { return engine_ ? engine_->droneX() : 0; }
int AppViewModel::droneY() const { return engine_ ? engine_->droneY() : 0; }
int AppViewModel::totalStars() const { return levelManager_ ? levelManager_->totalStars() : 0; }
int AppViewModel::completedCount() const { return levelManager_ ? levelManager_->completedCount() : 0; }

void AppViewModel::openLevel(int levelId) {
    if (!engine_ || !levelManager_) {
        return;
    }
    // Flush any pending debounced save for the previous level before switching.
    if (saveTimer_ && saveTimer_->isActive()) {
        saveTimer_->stop();
        if (saveManager_ && activeLevelId_ > 0) {
            saveManager_->saveScript(activeLevelId_, scriptText_);
        }
    }
    engine_->loadLevel(levelId);
    activeLevelId_ = levelId;
    scriptText_.clear();
    emit scriptTextChanged();
}

void AppViewModel::runOrPause() {
    if (!engine_) {
        return;
    }
    if (engine_->state() == GameEngine::Running) {
        engine_->pause();
        return;
    }
    if (scriptText_.trimmed().isEmpty()) {
        setConsoleLine(QStringLiteral("请先在编辑器中编写代码"));
        return;
    }
    engine_->loadScript(scriptText_);
    engine_->run();
}

void AppViewModel::stepOnce() {
    if (!engine_) {
        return;
    }
    if (scriptText_.trimmed().isEmpty()) {
        setConsoleLine(QStringLiteral("请先在编辑器中编写代码"));
        return;
    }
    engine_->loadScript(scriptText_);
    engine_->stepOnce();
}

void AppViewModel::resetLevel() {
    if (engine_) {
        engine_->reset();
    }
}

void AppViewModel::giveUp() {
    if (engine_) {
        engine_->giveUp();
    }
}

void AppViewModel::resetScriptToTutorial() {
    if (!engine_ || activeLevelId_ <= 0) {
        return;
    }
    saveScript(engine_->tutorialCode());
    setConsoleLine(QStringLiteral("已恢复本关教程脚本。"));
}

void AppViewModel::saveScript(const QString &text) {
    if (scriptText_ == text) {
        return;
    }
    scriptText_ = text;
    if (saveManager_ && activeLevelId_ > 0) {
        // Debounce disk writes: restart the timer so we only write once the
        // user stops typing for 500ms.
        saveTimer_->start();
    }
    if (engine_ && engine_->currentLevelId() == activeLevelId_) {
        engine_->loadScript(text);
    }
    emit scriptTextChanged();
}

QVariantMap AppViewModel::cellAt(int x, int y) const {
    auto *map = engine_ ? engine_->farmMap() : nullptr;
    return map ? map->getCellAt(x, y) : QVariantMap {};
}

int AppViewModel::nextUnlockedLevel() const {
    if (!levelManager_) {
        return 1;
    }
    for (int id = 1; id <= levelManager_->levelCount(); ++id) {
        if (levelManager_->isUnlocked(id) && levelManager_->getStars(id) == 0) {
            return id;
        }
    }
    return qMax(1, qMin(levelManager_->levelCount(), levelManager_->completedCount() + 1));
}

void AppViewModel::resetUiPreferences() {
    runSpeed_ = 1.0f;
    bgAnimations_ = true;
    autoShowHint_ = false;
    if (engine_) {
        engine_->setSpeed(runSpeed_);
    }
    savePreferences();
    emit uiPreferencesChanged();
}

void AppViewModel::resetAllProgress() {
    if (levelManager_) {
        levelManager_->resetProgress();
        emit levelsChanged();
    }
}

QVariantMap AppViewModel::level(int levelId) const {
    if (!levelManager_ || levelId <= 0) {
        return {};
    }
    QVariantMap item = levelManager_->getLevel(levelId);
    if (item.isEmpty()) {
        return item;
    }
    item.insert(QStringLiteral("unlocked"), levelManager_->isUnlocked(levelId));
    item.insert(QStringLiteral("stars"), levelManager_->getStars(levelId));
    item.insert(QStringLiteral("bestTime"), levelManager_->getBestTime(levelId));
    item.insert(QStringLiteral("clearCount"), levelManager_->getClearCount(levelId));
    return item;
}

QVariantMap AppViewModel::levelNewContent(int levelId) const {
    if (!levelManager_ || levelId <= 0) {
        return {};
    }
    return levelManager_->getLevelNewContent(levelId);
}

QString AppViewModel::failureHint(const QString &reason) const {
    if (reason == QStringLiteral("timeout")) {
        return QStringLiteral("超出关卡限制时间。尝试减少等待 tick 或优化移动路径。");
    }
    if (reason == QStringLiteral("script_completed")) {
        return QStringLiteral("脚本已经执行结束，但关卡目标尚未完成。通常需要补充 wait()、巡逻或收割阶段。");
    }
    if (reason == QStringLiteral("player_quit")) {
        return QStringLiteral("本次挑战已手动结束。");
    }
    return QStringLiteral("检查控制台日志、当前执行行和关卡目标。");
}

void AppViewModel::setRunSpeed(float speed) {
    const float next = qBound(0.1f, speed, 5.0f);
    if (qFuzzyCompare(runSpeed_, next)) {
        return;
    }
    runSpeed_ = next;
    if (engine_) {
        engine_->setSpeed(next);
    }
    savePreferences();
    emit uiPreferencesChanged();
}

void AppViewModel::setBgAnimations(bool enabled) {
    if (bgAnimations_ == enabled) {
        return;
    }
    bgAnimations_ = enabled;
    savePreferences();
    emit uiPreferencesChanged();
}

void AppViewModel::setAutoShowHint(bool enabled) {
    if (autoShowHint_ == enabled) {
        return;
    }
    autoShowHint_ = enabled;
    savePreferences();
    emit uiPreferencesChanged();
}

void AppViewModel::loadPreferences() {
    QSettings settings;
    runSpeed_ = settings.value(QStringLiteral("ui/runSpeed"), 1.0).toFloat();
    bgAnimations_ = settings.value(QStringLiteral("ui/bgAnimations"), true).toBool();
    autoShowHint_ = settings.value(QStringLiteral("ui/autoShowHint"), false).toBool();
}

void AppViewModel::savePreferences() const {
    QSettings settings;
    settings.setValue(QStringLiteral("ui/runSpeed"), runSpeed_);
    settings.setValue(QStringLiteral("ui/bgAnimations"), bgAnimations_);
    settings.setValue(QStringLiteral("ui/autoShowHint"), autoShowHint_);
}

void AppViewModel::refreshScriptFromStore() {
    if (!engine_ || !saveManager_ || activeLevelId_ <= 0) {
        return;
    }
    // Flush any pending debounced save so we don't overwrite the stored script
    // with stale content right before reading it.
    if (saveTimer_ && saveTimer_->isActive()) {
        saveTimer_->stop();
        saveManager_->saveScript(activeLevelId_, scriptText_);
    }
    const QString saved = saveManager_->loadScript(activeLevelId_);
    const QString next = saved;
    if (scriptText_ == next) {
        return;
    }
    scriptText_ = next;
    engine_->loadScript(scriptText_);
    emit scriptTextChanged();
}

void AppViewModel::setConsoleLine(const QString &line) {
    if (consoleLine_ == line) {
        return;
    }
    consoleLine_ = line;
    emit consoleLineChanged();
}
