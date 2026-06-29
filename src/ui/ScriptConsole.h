#ifndef SCRIPTCONSOLE_H
#define SCRIPTCONSOLE_H

#include <QWidget>

class GameState;
class QPlainTextEdit;

class ScriptConsole : public QWidget {
    Q_OBJECT
public:
    explicit ScriptConsole(GameState *state, QWidget *parent = nullptr);

    void appendOutput(const QString &text);
    void appendError(const QString &text);
    void clear();

private:
    GameState *m_gameState;
    QPlainTextEdit *m_output;
};

#endif // SCRIPTCONSOLE_H
