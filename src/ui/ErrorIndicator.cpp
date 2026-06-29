#include "ErrorIndicator.h"
#include "core/GameState.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QPainter>
#include <QToolTip>
#include <QEnterEvent>

ErrorIndicator::ErrorIndicator(GameState *state, QWidget *parent)
    : QWidget(parent)
    , m_gameState(state)
    , m_errorCount(0)
{
    setFixedSize(36, 36);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setStyleSheet("font-size: 22px; background: transparent;");
    m_iconLabel->setGeometry(0, 0, 36, 36);

    setStyleSheet("background: transparent;");
    hide();
}

void ErrorIndicator::addError(const QString &scriptName, int line,
                               const QString &message)
{
    m_errors.append(QString("%1 Line %2: %3").arg(scriptName).arg(line).arg(message));
    m_errorCount++;
    if (!isVisible()) show();
    update();
}

void ErrorIndicator::clearErrors() {
    m_errors.clear();
    m_errorCount = 0;
    hide();
}

void ErrorIndicator::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPolygonF triangle;
    triangle << QPointF(18, 5) << QPointF(32, 29) << QPointF(4, 29);
    painter.setBrush(QColor("#F1C96B"));
    painter.setPen(QPen(QColor("#9B7838"), 1.5));
    painter.drawPolygon(triangle);
    painter.setPen(QPen(QColor("#6B552F"), 2));
    painter.drawLine(QPointF(18, 13), QPointF(18, 21));
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#6B552F"));
    painter.drawEllipse(QPointF(18, 25), 1.5, 1.5);

    // Badge count
    if (m_errorCount > 0) {
        painter.setBrush(QColor("#F0A860"));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPointF(28, 8), 9, 9);
        painter.setPen(Qt::white);
        painter.setFont(QFont("sans-serif", 9, QFont::Bold));
        painter.drawText(QRectF(22, 2, 12, 12), Qt::AlignCenter,
                         QString::number(m_errorCount));
    }
}

void ErrorIndicator::enterEvent(QEnterEvent *) {
    QString tip = m_errors.join("\n");
    QToolTip::showText(QCursor::pos(), tip, this);
}

void ErrorIndicator::leaveEvent(QEvent *) {
    QToolTip::hideText();
}
