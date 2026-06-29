#include "syntaxhighlighter.h"
#include <QQuickTextDocument>
#include <QRegularExpression>

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

    // Strings
    static QRegularExpression strRe(R"(("[^"]*"|'[^']*'))");
    auto it = strRe.globalMatch(text);
    while (it.hasNext()) {
        auto m = it.next();
        setFormat(m.capturedStart(), m.capturedLength(), stringFormat_);
    }

    // Keywords
    static QStringList keywords = {"if","else","for","while","def","return","break","continue","in","not","and","or","True","False","None","range"};
    for (const auto &kw : keywords) {
        QRegularExpression kwRe("\\b" + kw + "\\b");
        auto kwIt = kwRe.globalMatch(text);
        while (kwIt.hasNext()) {
            auto m = kwIt.next();
            setFormat(m.capturedStart(), m.capturedLength(), keywordFormat_);
        }
    }

    // API functions
    static QStringList allFuncs = {"move","till","plant","harvest","water","fertilize","spray","wait","debug","get_pos","get_map_size","get_current","get_goals","get_energy","get_tick"};
    for (const auto &fn : allFuncs) {
        QRegularExpression fnRe("\\b" + fn + "\\b");
        auto fnIt = fnRe.globalMatch(text);
        while (fnIt.hasNext()) {
            auto m = fnIt.next();
            if (allowedFunctions_.isEmpty() || allowedFunctions_.contains(fn))
                setFormat(m.capturedStart(), m.capturedLength(), apiFuncFormat_);
            else
                setFormat(m.capturedStart(), m.capturedLength(), disabledFormat_);
        }
    }
}
