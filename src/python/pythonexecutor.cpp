#include "pythonexecutor.h"

#include <QMetaObject>
#include <QStringList>
#include <QVariant>

#include "core/cell.h"
#include "core/drone.h"
#include "core/farmmap.h"
#include "core/goalsystem.h"

#if defined(HAS_PYTHON) && defined(HAS_PYBIND11)
#pragma push_macro("slots")
#undef slots
#include <Python.h>
#include <pybind11/embed.h>
#include <pybind11/eval.h>
#include <pybind11/stl.h>
#pragma pop_macro("slots")

namespace py = pybind11;
#endif

namespace {
#if defined(HAS_PYTHON) && defined(HAS_PYBIND11)
std::once_flag s_pythonInitFlag;
bool s_pythonReady = false;
QString s_pythonInitError;

void ensurePythonRuntime() {
    std::call_once(s_pythonInitFlag, []() {
        try {
            py::initialize_interpreter();
            PyEval_SaveThread();
            s_pythonReady = true;
        } catch (const std::exception &err) {
            s_pythonInitError = QString::fromStdString(err.what());
        }
    });
}

std::pair<QString, int> extractPythonError(const py::error_already_set &err) {
    QString message = QString::fromStdString(err.what());
    int line = 0;

    try {
        const QString typeName = QString::fromStdString(py::str(err.type().attr("__name__")));
        const QString valueText = QString::fromStdString(py::str(err.value()));
        message = valueText.isEmpty() ? typeName
                                      : QStringLiteral("%1: %2").arg(typeName, valueText);

        py::module_ traceback = py::module_::import("traceback");
        py::list frames = traceback.attr("extract_tb")(err.trace());
        if (!frames.empty()) {
            py::object frame = frames[py::int_(py::len(frames) - 1)];
            line = py::int_(frame.attr("lineno"));
        }
    } catch (...) {
    }

    return {message, line};
}

py::object toPythonObject(const QVariant &value) {
    if (!value.isValid() || value.isNull()) {
        return py::none();
    }

    switch (value.typeId()) {
    case QMetaType::Bool:
        return py::bool_(value.toBool());
    case QMetaType::Int:
        return py::int_(value.toInt());
    case QMetaType::UInt:
        return py::int_(value.toUInt());
    case QMetaType::LongLong:
        return py::int_(value.toLongLong());
    case QMetaType::ULongLong:
        return py::int_(value.toULongLong());
    case QMetaType::Float:
    case QMetaType::Double:
        return py::float_(value.toDouble());
    case QMetaType::QString:
        return py::str(value.toString().toStdString());
    case QMetaType::QVariantList: {
        py::list items;
        const QVariantList list = value.toList();
        for (const QVariant &item : list) {
            items.append(toPythonObject(item));
        }
        return items;
    }
    case QMetaType::QVariantMap: {
        py::dict dict;
        const QVariantMap map = value.toMap();
        for (auto it = map.cbegin(); it != map.cend(); ++it) {
            dict[py::str(it.key().toStdString())] = toPythonObject(it.value());
        }
        return dict;
    }
    default:
        return py::str(value.toString().toStdString());
    }
}
#endif
} // namespace

PythonExecutor::PythonExecutor(QObject *parent) : QObject(parent) {
#if defined(HAS_PYTHON) && defined(HAS_PYBIND11)
    ensurePythonRuntime();
    interpreterReady_ = s_pythonReady;
#else
    interpreterReady_ = false;
#endif
}

PythonExecutor::~PythonExecutor() {
    stopWorkerThread();
}

void PythonExecutor::setContext(FarmMap *map, Drone *drone, GoalSystem *goals) {
    map_ = map;
    drone_ = drone;
    goals_ = goals;
}

void PythonExecutor::setAllowedFunctions(const QSet<QString> &funcs) {
    allowedFunctions_ = funcs;
}

void PythonExecutor::setAllowedSyntax(const QSet<QString> &syntax) {
    allowedSyntax_ = syntax;
}

void PythonExecutor::setAllowedCrops(const QSet<QString> &crops) {
    allowedCrops_ = crops;
}

void PythonExecutor::finishTick() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!actionInFlight_) {
            return;
        }
        actionInFlight_ = false;
    }
    cv_.notify_all();
}

void PythonExecutor::loadScript(const QString &code) {
    currentScript_ = code;
}

bool PythonExecutor::startScript() {
    stopWorkerThread();

    if (currentScript_.trimmed().isEmpty()) {
        emit errorOccurred(QStringLiteral("脚本为空，无法运行"), 0);
        return false;
    }

#if !(defined(HAS_PYTHON) && defined(HAS_PYBIND11))
    emit errorOccurred(QStringLiteral("当前构建未启用嵌入式 Python"), 0);
    return false;
#else
    ensurePythonRuntime();
    if (!s_pythonReady) {
        emit errorOccurred(s_pythonInitError.isEmpty()
                               ? QStringLiteral("Python 运行时初始化失败")
                               : s_pythonInitError,
                           0);
        return false;
    }

    try {
        py::gil_scoped_acquire gil;
        py::module_::import("builtins").attr("compile")(
            currentScript_.toStdString(),
            "__codefarm_script__",
            "exec");
    } catch (const py::error_already_set &err) {
        const auto [message, line] = extractPythonError(err);
        emit errorOccurred(message, line);
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingAction_.reset();
        scriptRunning_ = false;
        scriptCompleted_ = false;
        stopRequested_ = false;
        actionInFlight_ = false;
        actionResponseReady_ = false;
        lastActionResult_ = false;
        pendingError_ = false;
        pendingErrorMessage_.clear();
        pendingErrorLine_ = 0;
        currentTick_ = 0;
        lastReportedLine_ = -1;
        pythonThreadId_ = 0;
    }

    return startWorkerThread();
#endif
}

bool PythonExecutor::executeTick() {
    if (!map_ || !drone_) {
        return false;
    }

    if (currentScript_.trimmed().isEmpty()) {
        return true;
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        currentTick_++;
    }

    if (reportPendingError()) {
        return false;
    }

    PendingAction action;
    if (waitForAction(action)) {
        const bool result = dispatchAction(action);
        completeAction(result);
    }

    return !reportPendingError();
}

bool PythonExecutor::isScriptCompleted() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return scriptCompleted_;
}

void PythonExecutor::stopScript() {
    stopWorkerThread();
}

void PythonExecutor::resetState() {
    stopWorkerThread();

    std::lock_guard<std::mutex> lock(mutex_);
    pendingAction_.reset();
    scriptRunning_ = false;
    scriptCompleted_ = false;
    stopRequested_ = false;
    actionInFlight_ = false;
    actionResponseReady_ = false;
    lastActionResult_ = false;
    pendingError_ = false;
    pendingErrorMessage_.clear();
    pendingErrorLine_ = 0;
    currentTick_ = 0;
    lastReportedLine_ = -1;
}

bool PythonExecutor::waitForAction(PendingAction &action) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!pendingAction_.has_value() || actionInFlight_) {
        return false;
    }

    action = *pendingAction_;
    pendingAction_.reset();
    actionInFlight_ = true;
    cv_.notify_all();
    return true;
}

void PythonExecutor::completeAction(bool result) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        lastActionResult_ = result;
        actionResponseReady_ = true;
    }
    cv_.notify_all();
}

bool PythonExecutor::reportPendingError() {
    QString message;
    int line = 0;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!pendingError_) {
            return false;
        }
        message = pendingErrorMessage_;
        line = pendingErrorLine_;
        pendingError_ = false;
        pendingErrorMessage_.clear();
        pendingErrorLine_ = 0;
    }

    emit errorOccurred(message, line);
    return true;
}

void PythonExecutor::storePythonError(const QString &msg, int line) {
    bool shouldReport = false;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!pendingError_) {
            pendingError_ = true;
            pendingErrorMessage_ = msg;
            pendingErrorLine_ = line;
            shouldReport = true;
        }
        scriptRunning_ = false;
        scriptCompleted_ = false;
        stopRequested_ = true;
        pendingAction_.reset();
        actionInFlight_ = false;
        actionResponseReady_ = true;
    }

    cv_.notify_all();

    if (shouldReport) {
        QMetaObject::invokeMethod(this, [this]() { reportPendingError(); }, Qt::QueuedConnection);
    }
}

int PythonExecutor::currentPythonLine() const {
#if !(defined(HAS_PYTHON) && defined(HAS_PYBIND11))
    return 0;
#else
    if (!interpreterReady_) {
        return 0;
    }

    py::gil_scoped_acquire gil;
    PyFrameObject *frame = PyEval_GetFrame();
    if (!frame) {
        return 0;
    }
    return PyFrame_GetLineNumber(frame);
#endif
}

bool PythonExecutor::isFunctionAllowed(const QString &funcName) const {
    return allowedFunctions_.isEmpty() || allowedFunctions_.contains(funcName);
}

bool PythonExecutor::isCropAllowed(const QString &cropName) const {
    return allowedCrops_.isEmpty() || allowedCrops_.contains(cropName);
}

bool PythonExecutor::startWorkerThread() {
#if !(defined(HAS_PYTHON) && defined(HAS_PYBIND11))
    return false;
#else
    const QString script = currentScript_;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        scriptRunning_ = true;
        scriptCompleted_ = false;
        stopRequested_ = false;
    }

    workerThread_ = std::thread([this, script]() {
        try {
            py::gil_scoped_acquire gil;
            pythonThreadId_ = static_cast<unsigned long>(PyThread_get_thread_ident());

            py::module_ builtinsModule = py::module_::import("builtins");
            py::dict builtins;
            for (const char *name : {"abs", "all", "any", "bool", "dict", "enumerate", "float",
                                     "int", "len", "list", "max", "min", "range", "round",
                                     "sorted", "str", "sum", "tuple", "zip"}) {
                builtins[py::str(name)] = builtinsModule.attr(name);
            }

            builtins[py::str("__import__")] = py::cpp_function([](py::args, py::kwargs) -> py::object {
                throw std::runtime_error("import is disabled in Code Farm");
            });

            builtins[py::str("print")] = py::cpp_function([this](py::args args, py::kwargs kwargs) {
                QStringList parts;
                for (const auto &arg : args) {
                    parts.append(QString::fromStdString(py::str(arg)));
                }

                QString sep = QStringLiteral(" ");
                QString end = QStringLiteral("\n");
                if (kwargs.contains("sep")) {
                    sep = QString::fromStdString(py::str(kwargs[py::str("sep")]));
                }
                if (kwargs.contains("end")) {
                    end = QString::fromStdString(py::str(kwargs[py::str("end")]));
                }

                const QString text = (parts.join(sep) + end).trimmed();
                if (!text.isEmpty()) {
                    emit printOutput(text);
                }
            });

            auto queueAction = [this](ActionType type, const QString &argument = QString()) {
                PendingAction action{type, argument, 0};
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    if (stopRequested_) {
                        throw std::runtime_error("__CODEFARM_STOP__");
                    }
                }

                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [this]() {
                    return stopRequested_ || (!pendingAction_.has_value() && !actionInFlight_ &&
                                              !actionResponseReady_);
                });

                if (stopRequested_) {
                    throw std::runtime_error("__CODEFARM_STOP__");
                }

                pendingAction_ = action;
                lastActionResult_ = false;
                actionResponseReady_ = false;
                cv_.notify_all();

                cv_.wait(lock, [this]() {
                    return stopRequested_ || pendingError_ || (actionResponseReady_ && !actionInFlight_);
                });

                if (stopRequested_) {
                    throw std::runtime_error("__CODEFARM_STOP__");
                }
                if (pendingError_) {
                    throw std::runtime_error("__CODEFARM_ERROR__");
                }

                const bool result = lastActionResult_;
                actionResponseReady_ = false;
                lastActionResult_ = false;
                return result;
            };

            auto requireFunction = [this](const QString &name) {
                if (!isFunctionAllowed(name)) {
                    throw std::runtime_error(
                        QStringLiteral("当前关卡不允许调用 %1()").arg(name).toStdString());
                }
            };

            auto requireCrop = [this](const QString &crop) {
                if (!isCropAllowed(crop)) {
                    throw std::runtime_error(
                        QStringLiteral("当前关卡不允许种植 %1").arg(crop).toStdString());
                }
            };

            auto reportCurrentLine = [this]() {
                std::lock_guard<std::mutex> lock(mutex_);
                if (stopRequested_) {
                    throw std::runtime_error("__CODEFARM_STOP__");
                }
            };

            py::dict globals;
            globals[py::str("__builtins__")] = builtins;
            globals[py::str("__name__")] = py::str("__main__");

            globals[py::str("move")] = py::cpp_function([&queueAction, &requireFunction](const std::string &dir) {
                requireFunction(QStringLiteral("move"));
                return queueAction(ActionType::Move, QString::fromStdString(dir));
            });
            globals[py::str("till")] = py::cpp_function([&queueAction, &requireFunction]() {
                requireFunction(QStringLiteral("till"));
                return queueAction(ActionType::Till);
            });
            globals[py::str("plant")] = py::cpp_function(
                [&queueAction, &requireFunction, &requireCrop](const std::string &crop) {
                    const QString cropName = QString::fromStdString(crop);
                    requireFunction(QStringLiteral("plant"));
                    requireCrop(cropName);
                    return queueAction(ActionType::Plant, cropName);
                });
            globals[py::str("water")] = py::cpp_function([&queueAction, &requireFunction]() {
                requireFunction(QStringLiteral("water"));
                return queueAction(ActionType::Water);
            });
            globals[py::str("fertilize")] = py::cpp_function([&queueAction, &requireFunction]() {
                requireFunction(QStringLiteral("fertilize"));
                return queueAction(ActionType::Fertilize);
            });
            globals[py::str("harvest")] = py::cpp_function([&queueAction, &requireFunction]() {
                requireFunction(QStringLiteral("harvest"));
                return queueAction(ActionType::Harvest);
            });
            globals[py::str("spray")] = py::cpp_function([&queueAction, &requireFunction]() {
                requireFunction(QStringLiteral("spray"));
                return queueAction(ActionType::Spray);
            });
            globals[py::str("wait")] = py::cpp_function([&queueAction, &requireFunction]() {
                requireFunction(QStringLiteral("wait"));
                return queueAction(ActionType::Wait);
            });

            globals[py::str("get_pos")] = py::cpp_function([this, &requireFunction, &reportCurrentLine]() {
                requireFunction(QStringLiteral("get_pos"));
                reportCurrentLine();
                emit apiCalled(QStringLiteral("get_pos"), true);
                return py::make_tuple(drone_->x(), drone_->y());
            });
            globals[py::str("get_map_size")] = py::cpp_function([this, &requireFunction, &reportCurrentLine]() {
                requireFunction(QStringLiteral("get_map_size"));
                reportCurrentLine();
                emit apiCalled(QStringLiteral("get_map_size"), true);
                return py::make_tuple(map_->gridWidth(), map_->gridHeight());
            });
            globals[py::str("get_current")] = py::cpp_function([this, &requireFunction, &reportCurrentLine]() {
                requireFunction(QStringLiteral("get_current"));
                reportCurrentLine();
                emit apiCalled(QStringLiteral("get_current"), true);
                return toPythonObject(currentCellInfo());
            });
            globals[py::str("get_goals")] = py::cpp_function([this, &requireFunction, &reportCurrentLine]() {
                requireFunction(QStringLiteral("get_goals"));
                reportCurrentLine();
                emit apiCalled(QStringLiteral("get_goals"), true);
                return toPythonObject(goalsInfo());
            });
            globals[py::str("get_energy")] = py::cpp_function([this, &requireFunction, &reportCurrentLine]() {
                requireFunction(QStringLiteral("get_energy"));
                reportCurrentLine();
                emit apiCalled(QStringLiteral("get_energy"), true);
                QVariantMap energy{
                    {QStringLiteral("current"), drone_->energy()},
                    {QStringLiteral("max"), drone_->maxEnergy()},
                };
                return toPythonObject(energy);
            });
            globals[py::str("get_tick")] = py::cpp_function([this, &requireFunction, &reportCurrentLine]() {
                requireFunction(QStringLiteral("get_tick"));
                reportCurrentLine();
                emit apiCalled(QStringLiteral("get_tick"), true);
                std::lock_guard<std::mutex> lock(mutex_);
                return py::int_(currentTick_);
            });
            globals[py::str("debug")] = py::cpp_function([this, &requireFunction, &reportCurrentLine]() {
                requireFunction(QStringLiteral("debug"));
                reportCurrentLine();
                const QVariantMap cell = currentCellInfo();
                emit apiCalled(QStringLiteral("debug"), true);
                emit printOutput(
                    QStringLiteral("cell=(%1,%2) state=%3 crop=%4 water=%5 progress=%6 bug=%7")
                        .arg(cell.value(QStringLiteral("x")).toInt())
                        .arg(cell.value(QStringLiteral("y")).toInt())
                        .arg(cell.value(QStringLiteral("state")).toString(),
                             cell.value(QStringLiteral("crop")).toString(),
                             QString::number(cell.value(QStringLiteral("water")).toFloat(), 'f', 2),
                             QString::number(cell.value(QStringLiteral("progress")).toFloat(), 'f', 2),
                             cell.value(QStringLiteral("hasBug")).toBool() ? QStringLiteral("True")
                                                                            : QStringLiteral("False")));
                return toPythonObject(cell);
            });
            try {
                py::object code = builtinsModule.attr("compile")(
                    script.toStdString(),
                    "__codefarm_script__",
                    "exec");
                builtinsModule.attr("exec")(code, globals, globals);
            } catch (...) {
                throw;
            }

            {
                std::lock_guard<std::mutex> lock(mutex_);
                scriptRunning_ = false;
                scriptCompleted_ = true;
                stopRequested_ = false;
            }
            cv_.notify_all();
        } catch (const py::error_already_set &err) {
            const auto [message, line] = extractPythonError(err);
            bool stopped = false;
            {
                std::lock_guard<std::mutex> lock(mutex_);
                stopped = stopRequested_;
            }
            if (!stopped) {
                storePythonError(message, line > 0 ? line : currentPythonLine());
            }
        } catch (const std::exception &err) {
            bool stopped = false;
            {
                std::lock_guard<std::mutex> lock(mutex_);
                stopped = stopRequested_;
                scriptRunning_ = false;
                if (stopped) {
                    scriptCompleted_ = false;
                    stopRequested_ = false;
                }
            }

            const QString message = QString::fromStdString(err.what());
            if (!stopped && message != QStringLiteral("__CODEFARM_STOP__") &&
                message != QStringLiteral("__CODEFARM_ERROR__")) {
                storePythonError(message, currentPythonLine());
            }
        }
    });

    return true;
#endif
}

void PythonExecutor::stopWorkerThread(bool clearScript) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stopRequested_ = true;
        pendingAction_.reset();
        actionInFlight_ = false;
        actionResponseReady_ = true;
    }
    cv_.notify_all();

    if (workerThread_.joinable()) {
        workerThread_.join();
    }

    std::lock_guard<std::mutex> lock(mutex_);
    scriptRunning_ = false;
    scriptCompleted_ = false;
    stopRequested_ = false;
    actionInFlight_ = false;
    actionResponseReady_ = false;
    lastActionResult_ = false;
    pendingAction_.reset();
    lastReportedLine_ = -1;
    if (clearScript) {
        currentScript_.clear();
    }
}

bool PythonExecutor::dispatchAction(const PendingAction &action) {
    bool result = false;
    QString name;

    switch (action.type) {
    case ActionType::Move:
        name = QStringLiteral("move");
        result = executeMove(action.argument);
        break;
    case ActionType::Till:
        name = QStringLiteral("till");
        result = executeTill();
        break;
    case ActionType::Plant:
        name = QStringLiteral("plant");
        result = executePlant(action.argument);
        break;
    case ActionType::Water:
        name = QStringLiteral("water");
        result = executeWater();
        break;
    case ActionType::Fertilize:
        name = QStringLiteral("fertilize");
        result = executeFertilize();
        break;
    case ActionType::Harvest:
        name = QStringLiteral("harvest");
        result = executeHarvest();
        break;
    case ActionType::Spray:
        name = QStringLiteral("spray");
        result = executeSpray();
        break;
    case ActionType::Wait:
        name = QStringLiteral("wait");
        result = true;
        break;
    }

    emit apiCalled(name, result);
    return result;
}

QVariantMap PythonExecutor::currentCellInfo() const {
    QVariantMap cell = map_ ? map_->getCellAt(drone_->x(), drone_->y()) : QVariantMap{};
    cell.insert(QStringLiteral("x"), drone_ ? drone_->x() : 0);
    cell.insert(QStringLiteral("y"), drone_ ? drone_->y() : 0);
    return cell;
}

QVariantList PythonExecutor::goalsInfo() const {
    return goals_ ? goals_->goalsModel() : QVariantList{};
}

bool PythonExecutor::executeHarvest() {
    if (!map_ || !drone_) {
        return false;
    }

    const int x = drone_->x();
    const int y = drone_->y();
    Cell &cell = map_->cellAt(x, y);

    if (cell.state != CellState::Mature || cell.hasBug || !drone_->consumeEnergy(1.0f)) {
        return false;
    }

    const QString cropName = Cell::cropToString(cell.crop);
    cell.state = CellState::Empty;
    cell.crop = CropType::None;
    cell.progress = 0.0f;
    cell.water = 0.0f;
    cell.fertilized = false;
    cell.fertilizeTicks = 0;
    cell.hasBug = false;
    cell.bugImmuneTicks = 0;
    cell.bugTicks = 0;
    cell.tilledTicks = 0;

    if (goals_) {
        goals_->addHarvestCount(cropName);
    }
    map_->notifyCellChanged(x, y);
    return true;
}

bool PythonExecutor::executeTill() {
    if (!map_ || !drone_) {
        return false;
    }

    const int x = drone_->x();
    const int y = drone_->y();
    Cell &cell = map_->cellAt(x, y);

    if (cell.state != CellState::Empty || !drone_->consumeEnergy(2.0f)) {
        return false;
    }

    cell.state = CellState::Tilled;
    cell.crop = CropType::None;
    cell.progress = 0.0f;
    cell.water = 0.0f;
    cell.fertilized = false;
    cell.fertilizeTicks = 0;
    cell.hasBug = false;
    cell.bugImmuneTicks = 0;
    cell.bugTicks = 0;
    cell.tilledTicks = 0;
    map_->notifyCellChanged(x, y);
    return true;
}

bool PythonExecutor::executePlant(const QString &crop) {
    if (!map_ || !drone_) {
        return false;
    }

    const CropType cropType = Cell::cropFromString(crop);
    if (cropType == CropType::None || !isCropAllowed(crop)) {
        return false;
    }

    const int x = drone_->x();
    const int y = drone_->y();
    Cell &cell = map_->cellAt(x, y);

    if (cell.state != CellState::Tilled || !drone_->consumeEnergy(1.5f)) {
        return false;
    }

    cell.state = CellState::Planted;
    cell.crop = cropType;
    cell.progress = 0.0f;
    cell.water = 0.5f;
    cell.fertilized = false;
    cell.fertilizeTicks = 0;
    cell.hasBug = false;
    cell.bugImmuneTicks = 0;
    cell.bugTicks = 0;
    map_->notifyCellChanged(x, y);
    return true;
}

bool PythonExecutor::executeWater() {
    if (!map_ || !drone_) {
        return false;
    }

    const int x = drone_->x();
    const int y = drone_->y();
    Cell &cell = map_->cellAt(x, y);

    if (cell.state == CellState::Empty || !drone_->consumeEnergy(1.5f)) {
        return false;
    }

    cell.water = qMin(1.0f, cell.water + 0.35f);
    map_->notifyCellChanged(x, y);
    return true;
}

bool PythonExecutor::executeFertilize() {
    if (!map_ || !drone_) {
        return false;
    }

    const int x = drone_->x();
    const int y = drone_->y();
    Cell &cell = map_->cellAt(x, y);

    if ((cell.state != CellState::Planted && cell.state != CellState::Mature) ||
        !drone_->consumeEnergy(2.0f)) {
        return false;
    }

    cell.fertilized = true;
    cell.fertilizeTicks = 12;
    map_->notifyCellChanged(x, y);
    return true;
}

bool PythonExecutor::executeMove(const QString &dir) {
    if (!map_ || !drone_) {
        return false;
    }
    return drone_->move(dir, map_->gridWidth(), map_->gridHeight());
}

bool PythonExecutor::executeSpray() {
    if (!map_ || !drone_) {
        return false;
    }

    const int x = drone_->x();
    const int y = drone_->y();
    Cell &cell = map_->cellAt(x, y);

    if (cell.state == CellState::Empty || !drone_->consumeEnergy(1.5f)) {
        return false;
    }

    cell.hasBug = false;
    cell.bugTicks = 0;
    cell.bugImmuneTicks = 12;
    map_->notifyCellChanged(x, y);
    return true;
}
