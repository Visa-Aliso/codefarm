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
    void finishTick();

    void loadScript(const QString &code);
    bool startScript();
    bool executeTick();
    bool isScriptCompleted() const;
    void stopScript();
    void resetState();

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
    void completeAction(bool result);
    bool reportPendingError();
    void storePythonError(const QString &msg, int line);
    int currentPythonLine() const;
    bool isFunctionAllowed(const QString &funcName) const;
    bool isCropAllowed(const QString &cropName) const;
    bool startWorkerThread();
    void stopWorkerThread(bool clearScript = false);
    bool dispatchAction(const PendingAction &action);
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
    QString currentScript_;
    bool interpreterReady_ = false;
    bool scriptRunning_ = false;
    bool scriptCompleted_ = false;
    bool stopRequested_ = false;
    bool actionInFlight_ = false;
    bool actionResponseReady_ = false;
    bool lastActionResult_ = false;
    bool pendingError_ = false;
    QString pendingErrorMessage_;
    int pendingErrorLine_ = 0;
    int currentTick_ = 0;
    int lastReportedLine_ = -1;
    unsigned long pythonThreadId_ = 0;
    std::optional<PendingAction> pendingAction_;
    std::thread workerThread_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
};

#endif // PYTHONEXECUTOR_H
