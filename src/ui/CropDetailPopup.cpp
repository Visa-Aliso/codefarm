#include "CropDetailPopup.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QPainter>

CropDetailPopup::CropDetailPopup(QWidget *parent)
    : QWidget(parent, Qt::ToolTip | Qt::FramelessWindowHint)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setStyleSheet(R"(
        background: rgba(250, 247, 240, 0.90);
        border: 1px solid #D4C9B5;
        border-radius: 10px;
        padding: 10px;
        color: #4A3F35;
        font-size: 13px;
    )");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 10, 12, 10);
    layout->setSpacing(4);

    m_cropLabel = new QLabel(this);
    m_cropLabel->setStyleSheet("font-size: 16px; font-weight: bold; background: transparent;");
    m_growthLabel = new QLabel(this);
    m_growthLabel->setStyleSheet("background: transparent;");
    m_waterLabel = new QLabel(this);
    m_waterLabel->setStyleSheet("background: transparent;");
    m_fertLabel = new QLabel(this);
    m_fertLabel->setStyleSheet("background: transparent;");
    m_timeLabel = new QLabel(this);
    m_timeLabel->setStyleSheet("background: transparent;");

    layout->addWidget(m_cropLabel);
    layout->addWidget(m_growthLabel);
    layout->addWidget(m_waterLabel);
    layout->addWidget(m_fertLabel);
    layout->addWidget(m_timeLabel);

    hide();
}

void CropDetailPopup::showAt(const QPoint &globalPos,
                              const QString &cropName, int growthPct,
                              double water, double fertility, double timeLeft)
{
    m_cropLabel->setText(cropName);
    m_growthLabel->setText(QString("成长 %1%").arg(growthPct));
    m_waterLabel->setText(QString("水分 %1").arg(static_cast<int>(water)));
    m_fertLabel->setText(QString("肥力 %1").arg(static_cast<int>(fertility)));
    m_timeLabel->setText(QString("成熟剩余 %1秒").arg(static_cast<int>(timeLeft)));

    adjustSize();
    move(globalPos + QPoint(16, 16));
    show();
}

void CropDetailPopup::hidePopup() {
    hide();
}

void CropDetailPopup::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor(250, 247, 240, 230));
    painter.setPen(QPen(QColor("#D4C9B5"), 1));
    painter.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 10, 10);
}
