#include "syntaxhighlighter.h"
#include <QQuickTextDocument>
#include <QTextBlock>
#include <QTimer>

SyntaxHighlighter::SyntaxHighlighter(QObject *parent)
    : QSyntaxHighlighter(parent)
{
    keywordFormat_.setForeground(QColor("#8FC5FF"));
    keywordFormat_.setFontWeight(QFont::Bold);

    apiFuncFormat_.setForeground(QColor("#73D8A4"));
    apiFuncFormat_.setFontWeight(QFont::Bold);

    stringFormat_.setForeground(QColor("#F3C26B"));
    commentFormat_.setForeground(QColor("#94A79A"));
    commentFormat_.setFontItalic(true);

    disabledFormat_.setForeground(QColor("#667A6D"));
    disabledFormat_.setFontStrikeOut(true);

    // 新增：函数定义名（琥珀黄粗体）
    funcDefFormat_.setForeground(QColor("#F0C040"));
    funcDefFormat_.setFontWeight(QFont::Bold);

    // 新增：内置函数（青色）
    builtinFormat_.setForeground(QColor("#56B6C2"));

    // 新增：数字（暗绿）
    numberFormat_.setForeground(QColor("#B5CEA8"));

    // 新增：赋值变量（淡紫）
    varFormat_.setForeground(QColor("#C792EA"));

    // Pre-compile regex patterns
    strRe_.setPattern(R"(("[^"]*"|'[^']*'))");

    // range 从关键字列表中移除，归入内置函数类
    keywords_ = {"if","else","for","while","def","return","break","continue","in","not","and","or","True","False","None"};
    for (const auto &kw : keywords_)
        kwRes_.append(QRegularExpression("\\b" + kw + "\\b"));

    allFuncNames_ = {"move","till","plant","harvest","water","fertilize","spray","wait","debug","get_pos","get_map_size","get_current","get_goals","get_tick"};
    for (const auto &fn : allFuncNames_)
        fnRes_.append(QRegularExpression("\\b" + fn + "\\b"));

    // 内置函数（20个）—— 仅当紧跟 ( 时高亮，避免同名变量被误着色
    builtinNames_ = {"print","int","float","str","bool","range","tuple","len",
                     "enumerate","min","max","sum","dict","list","sorted","zip",
                     "all","any","round","abs"};
    QStringList builtinParts;
    for (const auto &b : builtinNames_)
        builtinParts.append("\\b" + b + "\\s*\\(");
    builtinRe_.setPattern(builtinParts.join('|'));

    // 函数定义名：def 后的标识符
    funcDefRe_.setPattern(R"(\bdef\s+(\w+))");

    // 数字：整数 / 小数
    numRe_.setPattern(R"(\b\d+(\.\d+)?\b)");

    // 赋值变量：w = 2 / count += 1（\+? 同时匹配 = 和 +=）
    varRe_.setPattern(R"(\b(\w+)\s*\+?=)");

    // 用户函数调用：identifier(（在 API/内置之后运行，仅匹配未着色的标识符）
    funcCallRe_.setPattern(R"(\b(\w+)\s*\()");

    // 循环变量：for name in
    loopVarRe_.setPattern(R"(\bfor\s+(\w+)\s+in\b)");
}

static bool isBuiltinOrApi(const QString &name,
                           const QStringList &apis,
                           const QStringList &builtins,
                           const QStringList &keywords) {
    return apis.contains(name) || builtins.contains(name) || keywords.contains(name);
}

void SyntaxHighlighter::setQuickDocument(QQuickTextDocument *doc) {
    quickDoc_ = doc;
    if (doc) {
        setDocument(doc->textDocument());
        if (!scanTimer_) {
            scanTimer_ = new QTimer(this);
            scanTimer_->setSingleShot(true);
            scanTimer_->setInterval(150);
            connect(scanTimer_, &QTimer::timeout, this, [this]() {
                updateVariableScan();
                rehighlight();
            });
        }
        connect(doc->textDocument(), &QTextDocument::contentsChanged,
                this, &SyntaxHighlighter::scheduleVariableScan);
        updateVariableScan();
    }
}

void SyntaxHighlighter::scheduleVariableScan() {
    if (scanTimer_)
        scanTimer_->start();
}

void SyntaxHighlighter::updateVariableScan() {
    if (!document()) return;
    knownVars_.clear();
    const auto &apis = allFuncNames_;
    const auto &bis = builtinNames_;
    const auto &kws = keywords_;
    // Scan entire document for variable names from assignments and loop vars
    for (QTextBlock block = document()->begin(); block.isValid(); block = block.next()) {
        const QString text = block.text();

        // Assignment: w = 2 / count += 1
        {
            auto it = varRe_.globalMatch(text);
            while (it.hasNext()) {
                auto m = it.next();
                const QString name = m.captured(1);
                if (!isBuiltinOrApi(name, apis, bis, kws))
                    knownVars_.insert(name);
            }
        }
        // Loop vars: for i in range(3)
        {
            auto it = loopVarRe_.globalMatch(text);
            while (it.hasNext()) {
                auto m = it.next();
                const QString name = m.captured(1);
                if (!isBuiltinOrApi(name, apis, bis, kws))
                    knownVars_.insert(name);
            }
        }
    }
    // Build a combined OR regex for the variable reference pass
    if (knownVars_.isEmpty()) {
        varRefRe_.setPattern(QString());  // invalid, skip matching
        return;
    }
    QStringList parts;
    for (const auto &v : knownVars_)
        parts.append("\\b" + v + "\\b");
    varRefRe_.setPattern(parts.join('|'));
}

void SyntaxHighlighter::setAllowedFunctions(const QSet<QString> &funcs) {
    allowedFunctions_ = funcs;
    rehighlight();
}

void SyntaxHighlighter::setAllowedSyntax(const QSet<QString> &syntax) {
    allowedSyntax_ = syntax;
    rehighlight();
}

static bool isPosInCommentOrString(int pos, int commentIdx, const QVector<QPair<int,int>> &strings) {
    if (commentIdx >= 0 && pos >= commentIdx) return true;
    for (const auto &r : strings) {
        if (pos >= r.first && pos < r.first + r.second) return true;
    }
    return false;
}

void SyntaxHighlighter::highlightBlock(const QString &text) {
    // Pre-compute comment and string regions so later passes skip them
    const int commentIdx = text.indexOf('#');
    QVector<QPair<int,int>> stringRanges;
    {
        auto sIt = strRe_.globalMatch(text);
        while (sIt.hasNext()) {
            auto m = sIt.next();
            stringRanges.append({m.capturedStart(), m.capturedLength()});
        }
    }

    // 1. Comments
    if (commentIdx >= 0)
        setFormat(commentIdx, text.length() - commentIdx, commentFormat_);

    // 2. Strings
    for (const auto &r : stringRanges)
        setFormat(r.first, r.second, stringFormat_);

    // 3. Keywords
    for (const auto &re : kwRes_) {
        auto kwIt = re.globalMatch(text);
        while (kwIt.hasNext()) {
            auto m = kwIt.next();
            if (isPosInCommentOrString(m.capturedStart(), commentIdx, stringRanges)) continue;
            setFormat(m.capturedStart(), m.capturedLength(), keywordFormat_);
        }
    }

    // 4. API functions
    for (int i = 0; i < fnRes_.size(); ++i) {
        auto fnIt = fnRes_[i].globalMatch(text);
        while (fnIt.hasNext()) {
            auto m = fnIt.next();
            if (isPosInCommentOrString(m.capturedStart(), commentIdx, stringRanges)) continue;
            if (allowedFunctions_.isEmpty() || allowedFunctions_.contains(allFuncNames_[i]))
                setFormat(m.capturedStart(), m.capturedLength(), apiFuncFormat_);
            else
                setFormat(m.capturedStart(), m.capturedLength(), disabledFormat_);
        }
    }

    // 5. Builtin function calls (skip if no '(' on line)
    if (text.contains('(')) {
        auto bIt = builtinRe_.globalMatch(text);
        while (bIt.hasNext()) {
            auto m = bIt.next();
            if (isPosInCommentOrString(m.capturedStart(), commentIdx, stringRanges)) continue;
            const int len = m.capturedLength();
            const QString matched = m.captured();
            int nameLen = matched.indexOf('(');
            if (nameLen < 0) nameLen = len;
            setFormat(m.capturedStart(), nameLen, builtinFormat_);
        }
    }

    // 6. Function definition names (after 'def')
    {
        auto fdIt = funcDefRe_.globalMatch(text);
        while (fdIt.hasNext()) {
            auto m = fdIt.next();
            if (isPosInCommentOrString(m.capturedStart(1), commentIdx, stringRanges)) continue;
            setFormat(m.capturedStart(1), m.capturedLength(1), funcDefFormat_);
        }
    }

    // 7. User function calls (skip if no '(' on line)
    if (text.contains('(')) {
        auto fcIt = funcCallRe_.globalMatch(text);
        while (fcIt.hasNext()) {
            auto m = fcIt.next();
            if (isPosInCommentOrString(m.capturedStart(1), commentIdx, stringRanges)) continue;
            const QString name = m.captured(1);
            if (!isBuiltinOrApi(name, allFuncNames_, builtinNames_, keywords_))
                setFormat(m.capturedStart(1), m.capturedLength(1), funcDefFormat_);
        }
    }

    // 8. Numbers
    {
        auto nIt = numRe_.globalMatch(text);
        while (nIt.hasNext()) {
            auto m = nIt.next();
            if (isPosInCommentOrString(m.capturedStart(), commentIdx, stringRanges)) continue;
            setFormat(m.capturedStart(), m.capturedLength(), numberFormat_);
        }
    }

    // 9. Assignment variables (name = / name +=) — skip if no '=' on line
    if (text.contains('=')) {
        auto vIt = varRe_.globalMatch(text);
        while (vIt.hasNext()) {
            auto m = vIt.next();
            if (isPosInCommentOrString(m.capturedStart(1), commentIdx, stringRanges)) continue;
            const QString name = m.captured(1);
            if (!isBuiltinOrApi(name, allFuncNames_, builtinNames_, keywords_)) {
                setFormat(m.capturedStart(1), m.capturedLength(1), varFormat_);
                if (!knownVars_.contains(name)) {
                    knownVars_.insert(name);
                    varRefDirty_ = true;
                }
            }
        }
    }

    // 10. Loop variables (for name in) — skip if no 'for' on line
    if (text.contains("for")) {
        auto lvIt = loopVarRe_.globalMatch(text);
        while (lvIt.hasNext()) {
            auto m = lvIt.next();
            if (isPosInCommentOrString(m.capturedStart(1), commentIdx, stringRanges)) continue;
            const QString name = m.captured(1);
            if (!isBuiltinOrApi(name, allFuncNames_, builtinNames_, keywords_)) {
                setFormat(m.capturedStart(1), m.capturedLength(1), varFormat_);
                if (!knownVars_.contains(name)) {
                    knownVars_.insert(name);
                    varRefDirty_ = true;
                }
            }
        }
    }

    // 11. Variable references (any occurrence of a known variable name)
    if (!knownVars_.isEmpty()) {
        if (varRefDirty_) {
            varRefDirty_ = false;
            QStringList parts;
            for (const auto &v : knownVars_)
                parts.append("\\b" + v + "\\b");
            varRefRe_.setPattern(parts.join('|'));
        }
        if (varRefRe_.isValid()) {
            auto vrIt = varRefRe_.globalMatch(text);
            while (vrIt.hasNext()) {
                auto m = vrIt.next();
                if (isPosInCommentOrString(m.capturedStart(), commentIdx, stringRanges)) continue;
                setFormat(m.capturedStart(), m.capturedLength(), varFormat_);
            }
        }
    }
}
