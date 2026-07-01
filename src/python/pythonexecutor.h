#ifndef PYTHONEXECUTOR_H
#define PYTHONEXECUTOR_H

#include <QObject>
#include <QSet>
#include <QVariantList>

#include <optional>
#include <thread>
#include <mutex>
#include <condition_variable>

class FarmMap;
class Drone;
class GoalSystem;

class PythonExecutor : public QObject {
    Q_OBJECT
public:
    explicit PythonExecutor(QObject *parent = nullptr);
    ~PythonExecutor();

    void setContext(FarmMap *map, Drone *drone, GoalSystem *goals);
    void setAllowedFunctions(const QSet<QString> &funcs);
    void setAllowedSyntax(const QSet<QString> &syntax);
    void setAllowedCrops(const QSet<QString> &crops);
    void setAllowedBuiltins(const QSet<QString> &builtins);
    void finishTick();

    // Exposed so the engine can synchronize tickUpdate with query-function reads.
    std::mutex& tickMutex() { return mutex_; }

    void loadScript(const QString &code);
    bool startScript();
    bool executeTick();
    bool isScriptCompleted() const;
    void stopScript();
    void resetState();

    // 检查当前脚本（玩家提交的代码）的 AST 是否使用了给定特性集合中的任一项。
    // 特性键：AST 语法键（for/while/.../listcomp/lambda/...）或游戏函数名（move/till/...）。
    // 返回 matched 集合（命中的特性），供引擎判 ★3。
    // 注意：此函数在 startScript() 时预计算，运行时无需 GIL。
    QSet<QString> codeUsesFeatures(const QSet<QString> &features) const;
    int currentTick() const { return currentTick_; }
    int actionTickCount() const { return actionTickCount_; }

signals:
    void errorOccurred(const QString &msg, int line);
    void lineExecuting(int line);
    void printOutput(const QString &text);
    void apiCalled(const QString &funcName, bool result);

private:
    enum class ActionType {
        Move,
        Till,
        Plant,
        Water,
        Fertilize,
        Harvest,
        Spray,
        Wait
    };

    struct PendingAction {
        ActionType type;
        QString argument;
        int line = 0;
    };

    bool waitForAction(PendingAction &action);
    void completeAction(bool result, const QString &error = QString());
    bool reportPendingError();
    void storePythonError(const QString &msg, int line);
    bool isFunctionAllowed(const QString &funcName) const;
    bool isCropAllowed(const QString &cropName) const;
    bool isBuiltinAllowed(const QString &name) const;
    bool startWorkerThread();
    void stopWorkerThread(bool clearScript = false);
    bool dispatchAction(const PendingAction &action, QString &errorOut);
    QVariantMap currentCellInfo() const;
    QVariantList goalsInfo() const;

    bool executeHarvest();
    bool executeTill();
    bool executePlant(const QString &crop);
    bool executeWater();
    bool executeFertilize();
    bool executeMove(const QString &dir);
    bool executeSpray();

    FarmMap *map_ = nullptr;
    Drone *drone_ = nullptr;
    GoalSystem *goals_ = nullptr;
    QSet<QString> allowedFunctions_;
    QSet<QString> allowedSyntax_;
    QSet<QString> allowedCrops_;
    QSet<QString> allowedBuiltins_;
    QString currentScript_;
    bool interpreterReady_ = false;
    bool scriptRunning_ = false;
    bool scriptCompleted_ = false;
    bool stopRequested_ = false;
    bool actionInFlight_ = false;
    bool actionResponseReady_ = false;
    bool lastActionResult_ = false;
    QString lastActionError_;
    bool pendingError_ = false;
    QString pendingErrorMessage_;
    int pendingErrorLine_ = 0;
    int currentTick_ = 0;
    int actionTickCount_ = 0;    // only counts ticks where an action was dispatched
    QSet<QString> scriptFeatures_;  // pre-computed AST features (set during startScript)
    int lastReportedLine_ = -1;
    unsigned long pythonThreadId_ = 0;
    std::optional<PendingAction> pendingAction_;
    std::thread workerThread_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
};

#endif // PYTHONEXECUTOR_H
