#pragma once

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

class CodeHighlighter final : public QSyntaxHighlighter {
    Q_OBJECT
public:
    explicit CodeHighlighter(QTextDocument *doc);

protected:
    void highlightBlock(const QString &text) override;

private:
    QTextCharFormat api;
    QTextCharFormat kw;
    QTextCharFormat str;
    QTextCharFormat comment;

    void apply(const QString &text, const QRegularExpression &rx, const QTextCharFormat &fmt);
};
