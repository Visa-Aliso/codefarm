#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include <QObject>
#include <QString>
#include <QVector>

class GameState;
class ScriptConsole;

class ScriptEngine : public QObject {
    Q_OBJECT
public:
    explicit ScriptEngine(GameState *state, ScriptConsole *console,
                           QObject *parent = nullptr);
    ~ScriptEngine();

    void executeScript(const QString &name, const QString &code);
    void stopExecution();
    bool isRunning() const { return m_running; }

    struct SyntaxError {
        int line;
        QString message;
    };
    QVector<SyntaxError> validateSyntax(const QString &code) const;

signals:
    void executionStarted(const QString &name);
    void executionFinished(const QString &name);
    void executionError(const QString &name, int line, const QString &msg);
    void scriptOutput(const QString &text);

private:
    void executePythonCode(const QString &name, const QString &code);
    QString buildSandboxGlobals() const;
    void registerBuiltins();

    GameState *m_gameState;
    ScriptConsole *m_console;
    bool m_running = false;
};

#endif // SCRIPTENGINE_H
