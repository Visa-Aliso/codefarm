#include "syntaxhighlighter.h"
#include <QQuickTextDocument>

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

    // Pre-compile regex patterns
    strRe_.setPattern(R"(("[^"]*"|'[^']*'))");

    static const QStringList keywords = {"if","else","for","while","def","return","break","continue","in","not","and","or","True","False","None","range"};
    for (const auto &kw : keywords)
        kwRes_.append(QRegularExpression("\\b" + kw + "\\b"));

    static const QStringList allFuncs = {"move","till","plant","harvest","water","fertilize","spray","wait","debug","get_pos","get_map_size","get_current","get_goals","get_tick"};
    for (const auto &fn : allFuncs)
        fnRes_.append(QRegularExpression("\\b" + fn + "\\b"));
}

void SyntaxHighlighter::setQuickDocument(QQuickTextDocument *doc) {
    quickDoc_ = doc;
    if (doc)
        setDocument(doc->textDocument());
}

void SyntaxHighlighter::setAllowedFunctions(const QSet<QString> &funcs) {
    allowedFunctions_ = funcs;
    rehighlight();
}

void SyntaxHighlighter::setAllowedSyntax(const QSet<QString> &syntax) {
    allowedSyntax_ = syntax;
    rehighlight();
}

void SyntaxHighlighter::highlightBlock(const QString &text) {
    // Comments
    int commentIdx = text.indexOf('#');
    if (commentIdx >= 0)
        setFormat(commentIdx, text.length() - commentIdx, commentFormat_);

    // Strings (pre-compiled)
    auto strIt = strRe_.globalMatch(text);
    while (strIt.hasNext()) {
        auto m = strIt.next();
        setFormat(m.capturedStart(), m.capturedLength(), stringFormat_);
    }

    // Keywords (pre-compiled)
    for (const auto &re : kwRes_) {
        auto kwIt = re.globalMatch(text);
        while (kwIt.hasNext()) {
            auto m = kwIt.next();
            setFormat(m.capturedStart(), m.capturedLength(), keywordFormat_);
        }
    }

    // API functions (pre-compiled)
    static const QStringList allFuncNames = {"move","till","plant","harvest","water","fertilize","spray","wait","debug","get_pos","get_map_size","get_current","get_goals","get_tick"};
    for (int i = 0; i < fnRes_.size(); ++i) {
        auto fnIt = fnRes_[i].globalMatch(text);
        while (fnIt.hasNext()) {
            auto m = fnIt.next();
            if (allowedFunctions_.isEmpty() || allowedFunctions_.contains(allFuncNames[i]))
                setFormat(m.capturedStart(), m.capturedLength(), apiFuncFormat_);
            else
                setFormat(m.capturedStart(), m.capturedLength(), disabledFormat_);
        }
    }
}
