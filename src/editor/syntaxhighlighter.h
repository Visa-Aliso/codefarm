#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QQuickTextDocument>
#include <QSet>
#include <QRegularExpression>
#include <QVector>

class SyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
    Q_PROPERTY(QQuickTextDocument* document READ quickDocument WRITE setQuickDocument)

public:
    explicit SyntaxHighlighter(QObject *parent = nullptr);

    QQuickTextDocument* quickDocument() const { return quickDoc_; }
    void setQuickDocument(QQuickTextDocument *doc);
    void setAllowedFunctions(const QSet<QString> &funcs);
    void setAllowedSyntax(const QSet<QString> &syntax);

protected:
    void highlightBlock(const QString &text) override;

private:
    QQuickTextDocument *quickDoc_ = nullptr;
    QSet<QString> allowedFunctions_;
    QSet<QString> allowedSyntax_;

    QTextCharFormat keywordFormat_;
    QTextCharFormat apiFuncFormat_;
    QTextCharFormat stringFormat_;
    QTextCharFormat commentFormat_;
    QTextCharFormat disabledFormat_;

    QRegularExpression strRe_;
    QVector<QRegularExpression> kwRes_;
    QVector<QRegularExpression> fnRes_;
};

#endif // SYNTAXHIGHLIGHTER_H
