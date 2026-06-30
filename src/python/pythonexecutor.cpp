#include "pythonexecutor.h"

#include <QMetaObject>
#include <QStringList>
#include <QVariant>
#include <QRegularExpression>

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
    // Only use err.what() — a pure C++ call that never touches Python objects.
    // Accessing err.type()/err.value()/err.trace() can SIGSEGV when pybind11
    // converts a C++ exception to a Python exception (invalid object state).
    const QString what = QString::fromStdString(err.what());
    // Extract the last "line N" occurrence (innermost frame) from the message.
    static const QRegularExpression re(QStringLiteral("line (\\d+)"));
    auto it = re.globalMatch(what);
    int line = 0;
    while (it.hasNext()) {
        line = it.next().captured(1).toInt();
    }
    return {what, line};
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

static QString normalizeIndentation(const QString &code) {
    // Replace tabs with 4 spaces to avoid Python TabError on mixed indentation
    QString result = code;
    result.replace(QStringLiteral("\t"), QStringLiteral("    "));
    return result;
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

    // Normalize tab characters to spaces to prevent TabError
    currentScript_ = normalizeIndentation(currentScript_);

    // Validate syntax on the main thread before spawning the worker. This
    // catches syntax errors early (with a clean error_already_set that is
    // safe to inspect) rather than letting the worker thread hit them while
    // holding the GIL, which causes GIL-state corruption on Python 3.14.
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

    // AST-based syntax enforcement: check that the script only uses syntax
    // constructs allowed by the current level's allowedSyntax set.
    if (!allowedSyntax_.isEmpty()) {
        try {
            py::gil_scoped_acquire gil;
            py::module_ astModule = py::module_::import("ast");
            py::object tree = astModule.attr("parse")(currentScript_.toStdString());

            // Map AST node types to allowedSyntax keys, then check.
            // We walk the tree via a Python helper that returns a set of
            // syntax-feature strings found in the AST.
            py::dict checkGlobals;
            checkGlobals[py::str("tree")] = tree;
            py::str checkScript(
                "import ast\n"
                "found = set()\n"
                "for node in ast.walk(tree):\n"
                "    if isinstance(node, ast.For): found.add('for')\n"
                "    elif isinstance(node, ast.While): found.add('while')\n"
                "    elif isinstance(node, ast.If):\n"
                "        found.add('if')\n"
                "        if node.orelse: found.add('else')\n"
                "    elif isinstance(node, ast.FunctionDef): found.add('def')\n"
                "    elif isinstance(node, ast.Assign): found.add('assign')\n"
                "    elif isinstance(node, ast.Break): found.add('break')\n"
                "    elif isinstance(node, ast.Continue): found.add('continue')\n"
                "    elif isinstance(node, ast.Return): found.add('return')\n"
                "    elif isinstance(node, ast.BoolOp):\n"
                "        if isinstance(node.op, ast.And): found.add('and')\n"
                "        elif isinstance(node.op, ast.Or): found.add('or')\n"
                "    elif isinstance(node, ast.UnaryOp):\n"
                "        if isinstance(node.op, ast.Not): found.add('not')\n"
            );
            py::module_::import("builtins").attr("exec")(checkScript, checkGlobals, checkGlobals);
            py::set foundSet = checkGlobals[py::str("found")];

            for (auto item : foundSet) {
                QString key = QString::fromStdString(py::str(item));
                // "expr", "call", "in", "range" are always allowed — skip.
                if (key == "expr" || key == "call" || key == "in" || key == "range") {
                    continue;
                }
                if (!allowedSyntax_.contains(key)) {
                    static const QHash<QString, QString> labels = {
                        {"for", "for 循环"}, {"while", "while 循环"},
                        {"if", "if 条件"}, {"else", "else 分支"},
                        {"def", "函数定义"}, {"assign", "变量赋值"},
                        {"break", "break"}, {"continue", "continue"},
                        {"return", "return"}, {"and", "and 运算"},
                        {"or", "or 运算"}, {"not", "not 运算"}
                    };
                    QString label = labels.value(key, key);
                    emit errorOccurred(
                        QStringLiteral("当前关卡不允许使用 %1").arg(label), 0);
                    return false;
                }
            }
        } catch (const py::error_already_set &err) {
            const auto [message, line] = extractPythonError(err);
            emit errorOccurred(message, line);
            return false;
        }
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
        lastActionError_.clear();
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
        QString error;
        const bool result = dispatchAction(action, error);
        completeAction(result, error);
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
    lastActionError_.clear();
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

void PythonExecutor::completeAction(bool result, const QString &error) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        lastActionResult_ = result;
        lastActionError_ = error;
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
                const QString errorMsg = lastActionError_;
                actionResponseReady_ = false;
                lastActionResult_ = false;
                lastActionError_.clear();

                // Log failure to console instead of throwing — lets Python code
                // continue running and handle the failure with if/else.
                if (!result && !errorMsg.isEmpty()) {
                    emit printOutput(QStringLiteral("⚠ %1").arg(errorMsg));
                }
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
                std::lock_guard<std::mutex> lock(mutex_);
                return py::make_tuple(drone_->x(), drone_->y());
            });
            globals[py::str("get_map_size")] = py::cpp_function([this, &requireFunction, &reportCurrentLine]() {
                requireFunction(QStringLiteral("get_map_size"));
                reportCurrentLine();
                emit apiCalled(QStringLiteral("get_map_size"), true);
                std::lock_guard<std::mutex> lock(mutex_);
                return py::make_tuple(map_->gridWidth(), map_->gridHeight());
            });
            globals[py::str("get_current")] = py::cpp_function([this, &requireFunction, &reportCurrentLine]() {
                requireFunction(QStringLiteral("get_current"));
                reportCurrentLine();
                emit apiCalled(QStringLiteral("get_current"), true);
                std::lock_guard<std::mutex> lock(mutex_);
                return toPythonObject(currentCellInfo());
            });
            globals[py::str("get_goals")] = py::cpp_function([this, &requireFunction, &reportCurrentLine]() {
                requireFunction(QStringLiteral("get_goals"));
                reportCurrentLine();
                emit apiCalled(QStringLiteral("get_goals"), true);
                std::lock_guard<std::mutex> lock(mutex_);
                return toPythonObject(goalsInfo());
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
                QVariantMap cell;
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    cell = currentCellInfo();
                }
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
            // NOTE: Line tracing via PyEval_SetTrace would go here, but
            // Python 3.14's opaque PyFrameObject + GIL interaction with
            // pybind11 causes a segfault on GIL release at lambda exit.
            // The lineExecuting signal is declared and wired through to the
            // UI but currently never emitted. Revisit with a 3.14-safe
            // approach (e.g. sys.settrace in pure Python with a wrapper
            // that calls back via a C extension capsule).

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
                storePythonError(message, line);
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
                storePythonError(message, 0);
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
    lastActionError_.clear();
    pendingAction_.reset();
    lastReportedLine_ = -1;
    if (clearScript) {
        currentScript_.clear();
    }
}

bool PythonExecutor::dispatchAction(const PendingAction &action, QString &errorOut) {
    bool result = false;
    QString name;

    switch (action.type) {
    case ActionType::Move:
        name = QStringLiteral("move");
        result = executeMove(action.argument);
        if (!result) {
            if (action.argument != "up" && action.argument != "down" &&
                action.argument != "left" && action.argument != "right") {
                errorOut = QStringLiteral("move() 方向参数无效，应为 \"up\"/\"down\"/\"left\"/\"right\"");
            } else {
                // 检查是否因为岩石无法移动
                int nx = drone_ ? drone_->x() : 0;
                int ny = drone_ ? drone_->y() : 0;
                if (action.argument == "up") ny--;
                else if (action.argument == "down") ny++;
                else if (action.argument == "left") nx--;
                else if (action.argument == "right") nx++;
                if (map_ && nx >= 0 && nx < map_->gridWidth() && ny >= 0 && ny < map_->gridHeight()) {
                    const Cell &target = map_->cellAt(nx, ny);
                    if (target.state == CellState::Rock) {
                        errorOut = QStringLiteral("move() 目标位置是岩石，无法移动");
                    } else {
                        errorOut = QStringLiteral("move() 无法移动到该位置（超出地图边界）");
                    }
                } else {
                    errorOut = QStringLiteral("move() 无法移动到该位置（超出地图边界）");
                }
            }
        }
        break;
    case ActionType::Till:
        name = QStringLiteral("till");
        result = executeTill();
        if (!result) {
            if (map_ && drone_) {
                const Cell &cell = map_->cellAt(drone_->x(), drone_->y());
                if (cell.state == CellState::Rock) {
                    errorOut = QStringLiteral("till() 当前格子是岩石，无法犁地");
                } else {
                    errorOut = QStringLiteral("till() 当前格子不是空地（状态: %1）").arg(Cell::stateToString(cell.state));
                }
            }
        }
        break;
    case ActionType::Plant:
        name = QStringLiteral("plant");
        result = executePlant(action.argument);
        if (!result) {
            if (map_ && drone_) {
                const Cell &cell = map_->cellAt(drone_->x(), drone_->y());
                if (Cell::cropFromString(action.argument) == CropType::None) {
                    errorOut = QStringLiteral("plant() 无效的作物类型: \"%1\"").arg(action.argument);
                } else if (!isCropAllowed(action.argument)) {
                    errorOut = QStringLiteral("plant() 当前关卡不允许种植: \"%1\"").arg(action.argument);
                } else {
                    errorOut = QStringLiteral("plant() 当前格子未开垦（状态: %1），请先 till()").arg(Cell::stateToString(cell.state));
                }
            }
        }
        break;
    case ActionType::Water:
        name = QStringLiteral("water");
        result = executeWater();
        if (!result) {
            if (map_ && drone_) {
                errorOut = QStringLiteral("water() 当前格子是空地，无法浇水");
            }
        }
        break;
    case ActionType::Fertilize:
        name = QStringLiteral("fertilize");
        result = executeFertilize();
        if (!result) {
            if (map_ && drone_) {
                const Cell &cell = map_->cellAt(drone_->x(), drone_->y());
                errorOut = QStringLiteral("fertilize() 当前格子没有作物（状态: %1）").arg(Cell::stateToString(cell.state));
            }
        }
        break;
    case ActionType::Harvest:
        name = QStringLiteral("harvest");
        result = executeHarvest();
        if (!result) {
            if (map_ && drone_) {
                const Cell &cell = map_->cellAt(drone_->x(), drone_->y());
                if (!cell.isHarvestable() && cell.crop != CropType::None) {
                    errorOut = QStringLiteral("harvest() 向日葵不可收割，它会持续为周围作物提供阳光加成");
                } else if (cell.state != CellState::Mature) {
                    errorOut = QStringLiteral("harvest() 作物未成熟（状态: %1，进度: %2）")
                        .arg(Cell::stateToString(cell.state))
                        .arg(cell.progress, 0, 'f', 2);
                } else if (cell.hasBug) {
                    errorOut = QStringLiteral("harvest() 当前格子有虫害，请先 spray()");
                }
            }
        }
        break;
    case ActionType::Spray:
        name = QStringLiteral("spray");
        result = executeSpray();
        if (!result) {
            if (map_ && drone_) {
                errorOut = QStringLiteral("spray() 当前格子是空地，无需杀虫");
            }
        }
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

    if (cell.state != CellState::Mature || cell.hasBug || !cell.isHarvestable()) {
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

    if (cell.state != CellState::Empty) {
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

    if (cell.state != CellState::Tilled) {
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

    if (cell.state == CellState::Empty || cell.state == CellState::Rock) {
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

    if (cell.state != CellState::Planted && cell.state != CellState::Mature) {
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

    // 计算目标位置，检查是否为岩石
    int nx = drone_->x();
    int ny = drone_->y();
    if (dir == "up") ny--;
    else if (dir == "down") ny++;
    else if (dir == "left") nx--;
    else if (dir == "right") nx++;
    else return false;

    if (nx < 0 || nx >= map_->gridWidth() || ny < 0 || ny >= map_->gridHeight()) {
        return false;
    }

    const Cell &target = map_->cellAt(nx, ny);
    if (target.state == CellState::Rock) {
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

    if (cell.state == CellState::Empty || cell.state == CellState::Rock) {
        return false;
    }

    cell.hasBug = false;
    cell.bugTicks = 0;
    cell.bugImmuneTicks = 12;
    map_->notifyCellChanged(x, y);
    return true;
}
