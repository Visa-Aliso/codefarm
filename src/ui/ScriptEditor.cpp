#include "ScriptEditor.h"
#include "ScriptConsole.h"
#include "ErrorIndicator.h"
#include "script/ScriptEngine.h"
#include "core/GameState.h"
#include "core/TechTree.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextCharFormat>
#include <QTextDocument>

// ---- PythonHighlighter ----

PythonHighlighter::PythonHighlighter(GameState *state, QTextDocument *parent)
    : QSyntaxHighlighter(parent)
    , m_gameState(state)
{
    // Keywords (purple — always allowed)
    m_keywordFormat.setForeground(QColor("#7B4F9D"));
    m_keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywords = {
        "\\bif\\b", "\\belse\\b", "\\belif\\b", "\\bfor\\b", "\\bwhile\\b",
        "\\bdef\\b", "\\breturn\\b", "\\bbreak\\b", "\\bcontinue\\b",
        "\\bclass\\b", "\\bimport\\b", "\\bfrom\\b", "\\bTrue\\b", "\\bFalse\\b",
        "\\bNone\\b", "\\band\\b", "\\bor\\b", "\\bnot\\b", "\\bin\\b",
        "\\bis\\b", "\\bwith\\b", "\\bas\\b", "\\btry\\b", "\\bexcept\\b",
        "\\braise\\b", "\\bpass\\b", "\\byield\\b"
    };
    for (auto &kw : keywords)
        m_rules.append({QRegularExpression(kw), m_keywordFormat});

    // Builtin functions (blue)
    m_builtinFormat.setForeground(QColor("#3B7DD8"));
    QStringList builtins = {
        "\\bprint\\b", "\\brange\\b", "\\blen\\b", "\\bint\\b", "\\bstr\\b",
        "\\bfloat\\b", "\\blist\\b", "\\bdict\\b", "\\bbool\\b", "\\btype\\b",
        "\\babs\\b", "\\bmin\\b", "\\bmax\\b", "\\bsum\\b", "\\bsorted\\b"
    };
    for (auto &b : builtins)
        m_rules.append({QRegularExpression(b), m_builtinFormat});

    // Unlocked game functions (green)
    m_unlockedFuncFormat.setForeground(QColor("#4A8F4A"));
    m_unlockedFuncFormat.setFontWeight(QFont::Bold);

    // Locked game functions (red strike-through)
    m_lockedFuncFormat.setForeground(QColor("#D04040"));
    m_lockedFuncFormat.setFontStrikeOut(true);

    // Strings (orange)
    m_stringFormat.setForeground(QColor("#D08030"));
    m_rules.append({QRegularExpression(R"("[^"]*")"), m_stringFormat});
    m_rules.append({QRegularExpression(R"('[^']*')"), m_stringFormat});

    // Numbers (blue-gray)
    m_numberFormat.setForeground(QColor("#5080A0"));
    m_rules.append({QRegularExpression(R"(\b\d+\.?\d*\b)"), m_numberFormat});

    // Comments (gray)
    m_commentFormat.setForeground(QColor("#9EA0A0"));
    m_commentFormat.setFontItalic(true);
    m_rules.append({QRegularExpression(R"(#.*$)"), m_commentFormat});
}

void PythonHighlighter::highlightBlock(const QString &text) {
    // Apply rules
    for (auto &rule : m_rules) {
        auto it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            auto match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // Highlight game functions (unlocked vs locked)
    QRegularExpression funcRe(R"(\b([a-z_][a-z0-9_]*)\s*\()");
    auto it = funcRe.globalMatch(text);
    while (it.hasNext()) {
        auto match = it.next();
        QString funcName = match.captured(1);
        // Skip already-highlighted builtins and keywords
        if (funcName == "print" || funcName == "range" || funcName == "len" ||
            funcName == "int" || funcName == "str" || funcName == "float" ||
            funcName == "list" || funcName == "dict" || funcName == "bool" ||
            funcName == "type" || funcName == "abs" || funcName == "min" ||
            funcName == "max" || funcName == "sum" || funcName == "sorted")
            continue;

        // Check if it's a known game function
        QSet<QString> gameFuncs = {"init", "plant", "harvest", "move", "move_up", "move_down",
                                      "move_left", "move_right", "get_pos", "till", "water",
                                      "fertilize", "debug", "scan", "nearest", "pathfind",
                                      "schedule", "wait"};
        if (gameFuncs.contains(funcName)) {
            if (m_gameState->isFunctionUnlocked(funcName)) {
                setFormat(match.capturedStart(1), match.capturedLength(1),
                          m_unlockedFuncFormat);
            } else {
                setFormat(match.capturedStart(1), match.capturedLength(1),
                          m_lockedFuncFormat);
            }
        }
    }

    // Highlight locked syntax
    struct SyntaxCheck { QString re; QString name; };
    QVector<SyntaxCheck> checks = {
        {R"(\bif\b)", "if"}, {R"(\belse\b)", "else"}, {R"(\bfor\b)", "for"},
        {R"(\bwhile\b)", "while"}, {R"(\bdef\b)", "def"}, {R"(\breturn\b)", "return"},
        {R"(\bbreak\b)", "break"}, {R"(\bcontinue\b)", "continue"},
    };
    for (auto &c : checks) {
        QRegularExpression re(c.re);
        auto match = re.match(text);
        if (match.hasMatch() && !m_gameState->isSyntaxUnlocked(c.name)) {
            QTextCharFormat fmt;
            fmt.setUnderlineStyle(QTextCharFormat::WaveUnderline);
            fmt.setUnderlineColor(QColor("#F0A860"));
            fmt.setToolTip(QString("'%1' 尚未解锁").arg(c.name));
            setFormat(match.capturedStart(), match.capturedLength(), fmt);
        }
    }
}

// ---- ScriptEditor ----

ScriptEditor::ScriptEditor(GameState *state, ScriptEngine *engine,
                             ScriptConsole *console, ErrorIndicator *errors,
                             QWidget *parent)
    : QWidget(parent)
    , m_gameState(state)
    , m_engine(engine)
    , m_console(console)
    , m_errorIndicator(errors)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(7);

    m_statusLabel = new QLabel("准备就绪", this);
    m_statusLabel->setStyleSheet("font-size: 11px; color: #78906C; background: transparent; padding-left: 4px;");
    layout->addWidget(m_statusLabel);

    m_editor = new QPlainTextEdit(this);
    m_editor->setStyleSheet(R"(
        QPlainTextEdit {
            background: #F7FCFB;
            border: 1px solid #C8D8CF;
            border-radius: 14px;
            padding: 10px;
            font-family: "JetBrains Mono", "Cascadia Mono", "DejaVu Sans Mono", monospace;
            font-size: 13px;
            color: #40503A;
            selection-background-color: #CFE5DD;
        }
    )");
    m_editor->setTabStopDistance(32);
    m_editor->setLineWrapMode(QPlainTextEdit::NoWrap);
    layout->addWidget(m_editor, 1);

    m_highlighter = new PythonHighlighter(m_gameState, m_editor->document());
    connect(m_editor, &QPlainTextEdit::textChanged, this, [this]() {
        QMetaObject::invokeMethod(m_highlighter, "rehighlight", Qt::QueuedConnection);
    });
    loadSingleScript("auto_farm.py", "# Code Farm 智能果园\n# init() 会重置田地和无人机，方便反复调试\n\ninit()\nplant(\"strawberry\")\nwater()\nwait(18000)\nharvest()\n");
}

void ScriptEditor::loadScriptsFromState()
{
    const auto &scripts = m_gameState->scripts();
    if (scripts.isEmpty()) {
        loadSingleScript("auto_farm.py", "# Code Farm 智能果园\n# 自动管理脚本\n\ninit()\nplant(\"strawberry\")\nwater()\nwait(18000)\nharvest()\n");
        return;
    }
    loadSingleScript(scripts.first().name, scripts.first().code);
}

void ScriptEditor::loadSingleScript(const QString &name, const QString &code)
{
    renameScript(name.isEmpty() ? QStringLiteral("script.py") : name);
    m_editor->setPlainText(code);
    m_statusLabel->setText("准备就绪");
}

void ScriptEditor::renameScript(const QString &name)
{
    m_scriptName = name.trimmed().isEmpty() ? QStringLiteral("script.py") : name.trimmed();
    emit titleChanged(this, m_scriptName);
}

void ScriptEditor::syncScriptsToState()
{
    auto &scripts = m_gameState->scripts();
    scripts.clear();
    scripts.append({currentName(), currentCode()});
}

void ScriptEditor::appendScripts(QVector<GameState::ScriptData> &scripts) const
{
    scripts.append({currentName(), currentCode()});
}

void ScriptEditor::addTab(const QString &name, const QString &code)
{
    loadSingleScript(name, code);
}

void ScriptEditor::runScript() {
    QString code = currentCode();
    QString name = currentName();

    if (code.trimmed().isEmpty()) return;

    m_console->clear();
    m_errorIndicator->clearErrors();
    m_statusLabel->setText("已发送给无人机调度队列");

    m_engine->executeScript(name, code);
}

void ScriptEditor::stopScript() {
    m_statusLabel->setText("已停止");
    m_engine->stopExecution();
}

QString ScriptEditor::currentCode() const {
    return m_editor ? m_editor->toPlainText() : QString();
}

QString ScriptEditor::currentName() const {
    return m_scriptName.isEmpty() ? QStringLiteral("script.py") : m_scriptName;
}
