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
    void updateVariableScan();
    void scheduleVariableScan();

    QQuickTextDocument *quickDoc_ = nullptr;
    QSet<QString> allowedFunctions_;
    QSet<QString> allowedSyntax_;
    QSet<QString> knownVars_;
    bool varRefDirty_ = false; // set true when new vars found, triggers regex rebuild
    QTimer *scanTimer_ = nullptr;

    QTextCharFormat keywordFormat_;
    QTextCharFormat apiFuncFormat_;
    QTextCharFormat stringFormat_;
    QTextCharFormat commentFormat_;
    QTextCharFormat disabledFormat_;
    QTextCharFormat funcDefFormat_;
    QTextCharFormat builtinFormat_;
    QTextCharFormat numberFormat_;
    QTextCharFormat varFormat_;

    QRegularExpression strRe_;
    QRegularExpression funcDefRe_;
    QRegularExpression funcCallRe_;
    QRegularExpression builtinRe_;
    QRegularExpression numRe_;
    QRegularExpression varRe_;
    QRegularExpression loopVarRe_;
    QRegularExpression varRefRe_;  // rebuilt each scan from knownVars_
    QVector<QRegularExpression> kwRes_;
    QVector<QRegularExpression> fnRes_;
    QStringList allFuncNames_;
    QStringList keywords_;
    QStringList builtinNames_;
};

#endif // SYNTAXHIGHLIGHTER_H
