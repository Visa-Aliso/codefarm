#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QWidget>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include "core/GameState.h"

class ScriptEngine;
class ScriptConsole;
class ErrorIndicator;
class QTabWidget;
class QPlainTextEdit;
class QPushButton;
class QLabel;

class PythonHighlighter;

class ScriptEditor : public QWidget {
    Q_OBJECT
public:
    explicit ScriptEditor(GameState *state, ScriptEngine *engine,
                           ScriptConsole *console, ErrorIndicator *errors,
                           QWidget *parent = nullptr);

    void addTab(const QString &name = QString(), const QString &code = QString());
    QString currentCode() const;
    QString currentName() const;
    void loadScriptsFromState();
    void syncScriptsToState();
    void loadSingleScript(const QString &name, const QString &code);
    void appendScripts(QVector<GameState::ScriptData> &scripts) const;
    void renameScript(const QString &name);
    void runScript();
    void stopScript();

signals:
    void titleChanged(ScriptEditor *editor, const QString &title);

private:
    GameState *m_gameState;
    ScriptEngine *m_engine;
    ScriptConsole *m_console;
    ErrorIndicator *m_errorIndicator;
    QPlainTextEdit *m_editor;
    QString m_scriptName;
    QLabel *m_statusLabel;
    PythonHighlighter *m_highlighter;
};

class PythonHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    explicit PythonHighlighter(GameState *state, QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct Rule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<Rule> m_rules;
    GameState *m_gameState;

    QTextCharFormat m_keywordFormat;
    QTextCharFormat m_unlockedFuncFormat;
    QTextCharFormat m_lockedFuncFormat;
    QTextCharFormat m_stringFormat;
    QTextCharFormat m_numberFormat;
    QTextCharFormat m_commentFormat;
    QTextCharFormat m_builtinFormat;
};

#endif // SCRIPTEDITOR_H
