#ifndef ERRORINDICATOR_H
#define ERRORINDICATOR_H

#include <QWidget>

class GameState;
class QLabel;

class ErrorIndicator : public QWidget {
    Q_OBJECT
public:
    explicit ErrorIndicator(GameState *state, QWidget *parent = nullptr);

    void addError(const QString &scriptName, int line, const QString &message);
    void clearErrors();

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    GameState *m_gameState;
    QLabel *m_iconLabel;
    QStringList m_errors;
    int m_errorCount = 0;
};

#endif // ERRORINDICATOR_H
