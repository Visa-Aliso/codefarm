#include "CodeHighlighter.h"

CodeHighlighter::CodeHighlighter(QTextDocument *doc) : QSyntaxHighlighter(doc)
{
    api.setForeground(QColor("#6b8e2f"));
    api.setFontWeight(QFont::DemiBold);

    kw.setForeground(QColor("#5f7fb2"));
    kw.setFontWeight(QFont::DemiBold);

    str.setForeground(QColor("#bc7a3c"));
    str.setFontWeight(QFont::Medium);

    comment.setForeground(QColor("#9aa18a"));
}

void CodeHighlighter::highlightBlock(const QString &text)
{
    apply(text, QRegularExpression("\\b(move|till|plant|water|fertilize|debug|harvest|get_pos|get_current|get_goals)\\b"), api);
    apply(text, QRegularExpression("\\b(if|else|for|while|def|return|break|continue|in|range|and|or|not)\\b"), kw);
    apply(text, QRegularExpression("\"[^\"]*\"|'[^']*'"), str);

    const int pos = text.indexOf('#');
    if (pos >= 0) setFormat(pos, text.size() - pos, comment);
}

void CodeHighlighter::apply(const QString &text, const QRegularExpression &rx, const QTextCharFormat &fmt)
{
    auto it = rx.globalMatch(text);
    while (it.hasNext()) {
        const auto match = it.next();
        setFormat(int(match.capturedStart()), int(match.capturedLength()), fmt);
    }
}
