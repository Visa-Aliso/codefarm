#include "ScriptConsole.h"
#include "core/GameState.h"
#include <QVBoxLayout>
#include <QPlainTextEdit>

ScriptConsole::ScriptConsole(GameState *state, QWidget *parent)
    : QWidget(parent)
    , m_gameState(state)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_output = new QPlainTextEdit(this);
    m_output->setReadOnly(true);
    m_output->setStyleSheet(R"(
        QPlainTextEdit {
            background: #F5F0E5;
            border: 1px solid #D4C9B5;
            border-radius: 8px;
            padding: 6px;
            font-family: "Consolas", "Courier New", monospace;
            font-size: 12px;
            color: #4A3F35;
        }
    )");
    layout->addWidget(m_output);
}

void ScriptConsole::appendOutput(const QString &text) {
    m_output->appendHtml(QString("<span style='color:#4A3F35;'>%1</span>").arg(text));
}

void ScriptConsole::appendError(const QString &text) {
    m_output->appendHtml(QString("<span style='color:#D04040; font-weight:bold;'>%1</span>").arg(text));
}

void ScriptConsole::clear() {
    m_output->clear();
}
