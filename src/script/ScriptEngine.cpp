#include "ScriptEngine.h"
#include "core/GameState.h"
#include "core/Drone.h"
#include "core/TechTree.h"
#include "core/CropData.h"
#include "core/PathFinder.h"
#include "script/ScriptAPI.h"
#include "ui/ScriptConsole.h"

#if HAS_PYBIND11
// Qt defines 'slots' as a macro; Python's object.h uses 'slots' as a member name.
#pragma push_macro("slots")
#undef slots
#include <pybind11/embed.h>
#include <pybind11/stl.h>
#pragma pop_macro("slots")
namespace py = pybind11;
#endif

#include <QRegularExpression>
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

ScriptEngine::ScriptEngine(GameState *state, ScriptConsole *console,
                             QObject *parent)
    : QObject(parent)
    , m_gameState(state)
    , m_console(console)
{
}

ScriptEngine::~ScriptEngine() {
    stopExecution();
}

void ScriptEngine::executeScript(const QString &name, const QString &code) {
    if (m_running) return;

    // Validate syntax first
    auto errors = validateSyntax(code);
    if (!errors.isEmpty()) {
        for (auto &e : errors)
            emit executionError(name, e.line, e.message);
        m_console->appendError(QString("脚本 '%1' 包含未解锁的语法").arg(name));
        return;
    }

    m_running = true;
    emit executionStarted(name);
    m_console->appendOutput(QString(">>> 运行脚本: %1").arg(name));

    executePythonCode(name, code);

    m_running = false;
    emit executionFinished(name);
    m_console->appendOutput(QString("<<< 脚本 '%1' 执行完毕").arg(name));
}

void ScriptEngine::executePythonCode(const QString &name, const QString &code) {
#if HAS_PYBIND11
    try {
        if (!Py_IsInitialized()) {
            py::initialize_interpreter();
        }

        py::dict locals;

        ScriptAPI api(m_gameState);
        auto toPyDict = [](const QJsonObject &result) {
            py::dict d;
            for (auto it = result.begin(); it != result.end(); ++it) {
                const auto key = it.key().toStdString();
                const QJsonValue value = it.value();
                if (value.isBool()) d[key.c_str()] = py::bool_(value.toBool());
                else if (value.isDouble()) d[key.c_str()] = value.toDouble();
                else d[key.c_str()] = py::str(value.toString().toStdString());
            }
            return d;
        };

        locals["init"] = py::cpp_function([&api, toPyDict]() -> py::dict {
            return toPyDict(api.init());
        });

        // plant(), plant("crop") or plant(x, y)
        if (m_gameState->isFunctionUnlocked("plant")) {
            locals["plant"] = py::cpp_function([&api, toPyDict](py::args args) -> py::dict {
                if (args.size() == 2)
                    return toPyDict(api.plant(args[0].cast<int>(), args[1].cast<int>()));
                if (args.size() == 1)
                    return toPyDict(api.plantCurrent(QString::fromStdString(py::str(args[0]).cast<std::string>())));
                return toPyDict(api.plantCurrent());
            });
        }

        // harvest() or harvest(x, y)
        if (m_gameState->isFunctionUnlocked("harvest")) {
            locals["harvest"] = py::cpp_function([&api, toPyDict](py::args args) -> py::dict {
                if (args.size() == 2)
                    return toPyDict(api.harvest(args[0].cast<int>(), args[1].cast<int>()));
                return toPyDict(api.harvestCurrent());
            });
        }

        // move(x, y) uses the first drone in early-game scripts.
        if (m_gameState->isFunctionUnlocked("move")) {
            locals["move"] = py::cpp_function([&api, toPyDict](int x, int y) -> py::dict {
                return toPyDict(api.move(0, x, y));
            });
        }
        if (m_gameState->isFunctionUnlocked("move_up")) locals["move_up"] = py::cpp_function([&api, toPyDict]() { return toPyDict(api.moveDir(0, -1)); });
        if (m_gameState->isFunctionUnlocked("move_down")) locals["move_down"] = py::cpp_function([&api, toPyDict]() { return toPyDict(api.moveDir(0, 1)); });
        if (m_gameState->isFunctionUnlocked("move_left")) locals["move_left"] = py::cpp_function([&api, toPyDict]() { return toPyDict(api.moveDir(-1, 0)); });
        if (m_gameState->isFunctionUnlocked("move_right")) locals["move_right"] = py::cpp_function([&api, toPyDict]() { return toPyDict(api.moveDir(1, 0)); });
        if (m_gameState->isFunctionUnlocked("get_pos")) locals["get_pos"] = py::cpp_function([&api, toPyDict]() { return toPyDict(api.getPos()); });
        if (m_gameState->isFunctionUnlocked("till")) locals["till"] = py::cpp_function([&api, toPyDict]() { return toPyDict(api.tillCurrent()); });

        // water() or water(x, y)
        if (m_gameState->isFunctionUnlocked("water")) {
            locals["water"] = py::cpp_function([&api, toPyDict](py::args args) -> py::dict {
                if (args.size() == 2) return toPyDict(api.water(args[0].cast<int>(), args[1].cast<int>()));
                return toPyDict(api.waterCurrent());
            });
        }

        // fertilize(x, y)
        if (m_gameState->isFunctionUnlocked("fertilize")) {
            locals["fertilize"] = py::cpp_function([&api, toPyDict](py::args args) -> py::dict {
                if (args.size() == 2) return toPyDict(api.fertilize(args[0].cast<int>(), args[1].cast<int>()));
                return toPyDict(api.fertilizeCurrent());
            });
        }

        // debug(x, y)
        if (m_gameState->isFunctionUnlocked("debug")) {
            locals["debug"] = py::cpp_function([&api](int x, int y) -> py::dict {
                QJsonObject result = api.debugPlot(x, y);
                py::dict d;
                d["ok"] = py::bool_(result["ok"].toBool());
                return d;
            });
        }

        // scan(x, y)
        if (m_gameState->isFunctionUnlocked("scan")) {
            locals["scan"] = py::cpp_function([&api, toPyDict](py::args args) -> py::dict {
                if (args.size() == 2) return toPyDict(api.scan(args[0].cast<int>(), args[1].cast<int>()));
                return toPyDict(api.scanCurrent());
            });
        }

        // nearest(droneId, state)
        if (m_gameState->isFunctionUnlocked("nearest")) {
            locals["nearest"] = py::cpp_function([&api](int droneId, const std::string &state) -> py::dict {
                QJsonObject result = api.nearest(droneId, QString::fromStdString(state));
                py::dict d;
                d["x"] = result["x"].toInt(-1);
                d["y"] = result["y"].toInt(-1);
                d["ok"] = result["ok"].toBool();
                return d;
            });
        }

        // pathfind(x1, y1, x2, y2)
        if (m_gameState->isFunctionUnlocked("pathfind")) {
            locals["pathfind"] = py::cpp_function([&api](int x1, int y1, int x2, int y2) -> py::dict {
                QJsonObject result = api.pathfind(x1, y1, x2, y2);
                QJsonArray pathArr = result["path"].toArray();
                py::list pathList;
                for (auto val : pathArr) {
                    QJsonObject pt = val.toObject();
                    py::dict pd;
                    pd["x"] = pt["x"].toInt();
                    pd["y"] = pt["y"].toInt();
                    pathList.append(pd);
                }
                py::dict d;
                d["path"] = pathList;
                d["ok"] = result["ok"].toBool();
                return d;
            });
        }

        // schedule(name, delay)
        if (m_gameState->isFunctionUnlocked("schedule")) {
            locals["schedule"] = py::cpp_function([](const std::string &, int) -> py::dict {
                py::dict d;
                d["ok"] = py::bool_(true);
                return d;
            });
        }

        locals["wait"] = py::cpp_function([&api, toPyDict](int ms) -> py::dict {
            return toPyDict(api.wait(ms));
        });

        // Capture print() output
        locals["print"] = py::cpp_function([this](py::args args) {
            QStringList parts;
            for (auto &arg : args) {
                parts.append(QString::fromStdString(py::str(arg).cast<std::string>()));
            }
            QString output = parts.join(" ");
            m_console->appendOutput(output);
            emit scriptOutput(output);
        });

        // Execute the code
        py::exec(code.toStdString(), py::globals(), locals);

    } catch (py::error_already_set &e) {
        m_console->appendError(QString("Python 错误: %1").arg(e.what()));
        emit executionError(name, 0, e.what());
    } catch (std::exception &e) {
        m_console->appendError(QString("错误: %1").arg(e.what()));
        emit executionError(name, 0, e.what());
    }
#else
    ScriptAPI api(m_gameState);
    const QStringList lines = code.split('\n');
    QRegularExpression coordCallRe(R"(^\s*(plant|harvest|move|water|fertilize|debug)\s*\(\s*(\d+)\s*,\s*(\d+)\s*\)\s*$)");
    QRegularExpression simpleCallRe(R"(^\s*(init|plant|harvest|till|water|fertilize|move_up|move_down|move_left|move_right)\s*\(\s*(?:['\"]?([a-zA-Z_\u4e00-\u9fa5]+)['\"]?)?\s*\)\s*$)");
    QRegularExpression waitRe(R"(^\s*wait\s*\(\s*(\d+)\s*\)\s*$)");
    for (int i = 0; i < lines.size(); ++i) {
        QJsonObject result;
        auto coord = coordCallRe.match(lines[i]);
        auto simple = simpleCallRe.match(lines[i]);
        auto wait = waitRe.match(lines[i]);
        if (coord.hasMatch()) {
            const QString fn = coord.captured(1);
            const int x = coord.captured(2).toInt();
            const int y = coord.captured(3).toInt();
            if (fn == "plant") result = api.plant(x, y);
            else if (fn == "harvest") result = api.harvest(x, y);
            else if (fn == "move") result = api.move(0, x, y);
            else if (fn == "water") result = api.water(x, y);
            else if (fn == "fertilize") result = api.fertilize(x, y);
            else if (fn == "debug") result = api.debugPlot(x, y);
        } else if (simple.hasMatch()) {
            const QString fn = simple.captured(1);
            if (fn == "init") result = api.init();
            else if (fn == "plant") result = api.plantCurrent(simple.captured(2).isEmpty() ? "strawberry" : simple.captured(2));
            else if (fn == "harvest") result = api.harvestCurrent();
            else if (fn == "till") result = api.tillCurrent();
            else if (fn == "water") result = api.waterCurrent();
            else if (fn == "fertilize") result = api.fertilizeCurrent();
            else if (fn == "move_up") result = api.moveDir(0, -1);
            else if (fn == "move_down") result = api.moveDir(0, 1);
            else if (fn == "move_left") result = api.moveDir(-1, 0);
            else if (fn == "move_right") result = api.moveDir(1, 0);
        } else if (wait.hasMatch()) {
            result = api.wait(wait.captured(1).toInt());
        } else {
            continue;
        }

        if (!result["ok"].toBool()) {
            const QString error = result["error"].toString("操作失败");
            m_console->appendError(QString("Line %1: %2").arg(i + 1).arg(error));
            emit executionError(name, i + 1, error);
        }
    }
    m_console->appendOutput(QString("[模拟模式] 已将脚本 '%1' 中的基础指令加入无人机队列").arg(name));
    m_console->appendOutput("提示: 安装 pybind11-dev 以启用 Python 脚本功能");
#endif
}

void ScriptEngine::stopExecution() {
    m_running = false;
    // Note: Python doesn't support cooperative cancellation easily.
    // Scripts run to completion or raise KeyboardInterrupt.
}

QVector<ScriptEngine::SyntaxError> ScriptEngine::validateSyntax(
    const QString &code) const
{
    QVector<SyntaxError> errors;

    struct Check {
        QString pattern;
        QString syntaxName;
    };

    QVector<Check> checks = {
        {R"(\bif\b)", "if"},
        {R"(\belse\b)", "else"},
        {R"(\bfor\b)", "for"},
        {R"(\brange\b)", "range"},
        {R"(\bwhile\b)", "while"},
        {R"(\bbreak\b)", "break"},
        {R"(\bcontinue\b)", "continue"},
        {R"(\bdef\b)", "def"},
        {R"(\breturn\b)", "return"},
    };

    QStringList lines = code.split('\n');
    for (int i = 0; i < lines.size(); ++i) {
        QString stripped = lines[i];
        // Remove comments
        int comment = stripped.indexOf('#');
        if (comment >= 0) stripped = stripped.left(comment);
        // Remove string literals
        stripped.remove(QRegularExpression(R"("[^"]*")"));
        stripped.remove(QRegularExpression(R"('[^']*')"));

        for (auto &check : checks) {
        QRegularExpression re(check.pattern);
            if (re.match(stripped).hasMatch()) {
                if (!m_gameState->isSyntaxUnlocked(check.syntaxName)) {
                    errors.append({i + 1, QString("'%1' 尚未解锁").arg(check.syntaxName)});
                }
            }
        }

        // Check for list/dict literals
        if (!m_gameState->isSyntaxUnlocked("list") &&
            stripped.contains(QRegularExpression(R"(\[.*\])"))) {
            errors.append({i + 1, "'list' 尚未解锁"});
        }
        if (!m_gameState->isSyntaxUnlocked("dict") &&
            stripped.contains(QRegularExpression(R"(\{.*\})"))) {
            errors.append({i + 1, "'dict' 尚未解锁"});
        }
    }

    // Check for unlocked functions
    QRegularExpression funcRe(R"(\b([a-z_]+)\s*\()");
    auto it = funcRe.globalMatch(code);
    while (it.hasNext()) {
        auto match = it.next();
        QString funcName = match.captured(1);
        // Skip built-in Python functions
        QSet<QString> builtins = {"print", "range", "len", "int", "str", "float",
                                   "list", "dict", "bool", "type", "abs", "min", "max",
                                   "sum", "round", "sorted", "enumerate", "zip", "map",
                                   "filter", "any", "all", "isinstance", "set", "tuple"};
        if (builtins.contains(funcName)) continue;
        if (!m_gameState->isFunctionUnlocked(funcName)) {
            int lineNum = code.left(match.capturedStart()).count('\n') + 1;
            errors.append({lineNum, QString("'%1()' 尚未解锁").arg(funcName)});
        }
    }

    return errors;
}
