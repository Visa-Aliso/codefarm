#include "InfoPanel.h"
#include "core/GameState.h"
#include "core/Drone.h"
#include "core/TechTree.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QPainter>

InfoPanel::InfoPanel(GameState *state, QWidget *parent)
    : QWidget(parent)
    , m_gameState(state)
{
    setupUI();
    connect(m_gameState, &GameState::goldChanged, this, &InfoPanel::refresh);
}

void InfoPanel::setupUI()
{
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_StyledBackground, false);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(14, 8, 14, 8);
    layout->setSpacing(0);

    m_goldLabel = makeValueLabel(layout);
    m_levelLabel = nullptr;
    m_weatherLabel = nullptr;
    m_mapLabel = nullptr;
    m_droneLabel = nullptr;

    setMinimumWidth(128);
    setMaximumHeight(48);
}

QLabel *InfoPanel::makeValueLabel(QHBoxLayout *layout)
{
    auto *label = new QLabel(this);
    label->setStyleSheet("color: #3D4C35; background: transparent; font-size: 18px; font-weight: 900; padding-left: 34px;");
    label->setMinimumWidth(112);
    layout->addWidget(label);
    return label;
}

void InfoPanel::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const QRectF rect = this->rect().adjusted(2, 3, -2, -3);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 250, 236, 230));
    painter.drawRoundedRect(rect, 18, 18);
    painter.setBrush(QColor("#E7C56D"));
    painter.drawEllipse(QPointF(rect.left() + 21, rect.center().y()), 13, 13);
    painter.setPen(QPen(QColor(75, 85, 65), 1));
    painter.setFont(QFont("LXGW WenKai", 10, QFont::Black));
    painter.drawText(QRectF(rect.left() + 8, rect.center().y() - 13, 26, 26), Qt::AlignCenter, "G");
}

void InfoPanel::refresh()
{
    m_goldLabel->setText(QString("金币 %1").arg(m_gameState->gold()));
}
