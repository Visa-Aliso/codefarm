#include "MenuScreen.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QPropertyAnimation>
#include <QTimer>
#include <QResizeEvent>
#include <QRandomGenerator>
#include <QDateTime>
#include <QGraphicsOpacityEffect>

MenuScreen::MenuScreen(QWidget *parent)
    : QWidget(parent)
    , m_cardWidget(nullptr)
    , m_cloudTimer(nullptr)
{
    setupUI();
}

MenuScreen::~MenuScreen()
{
    stopCloudAnimations();
}

void MenuScreen::setupUI()
{
    // Card widget — centered in the screen
    m_cardWidget = new QWidget(this);
    m_cardWidget->setObjectName("menuCard");
    m_cardWidget->setFixedWidth(420);
    m_cardWidget->setStyleSheet(R"(
        #menuCard {
            background: rgba(250, 247, 240, 0.92);
            border-radius: 24px;
            border: 2px solid #D4C9B5;
        }
    )");

    auto *cardLayout = new QVBoxLayout(m_cardWidget);
    cardLayout->setContentsMargins(40, 36, 40, 36);
    cardLayout->setSpacing(16);

    // Title
    auto *title = new QLabel("Code Farm", m_cardWidget);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(R"(
        font-size: 36px;
        font-weight: bold;
        color: #4A3F35;
        background: transparent;
    )");

    // Subtitle
    auto *subtitle = new QLabel("智能果园", m_cardWidget);
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet(R"(
        font-size: 18px;
        color: #8B7355;
        background: transparent;
        margin-bottom: 12px;
    )");

    cardLayout->addWidget(title);
    cardLayout->addWidget(subtitle);

    // Buttons
    auto makeButton = [this](const QString &text, QWidget *parent) -> QPushButton * {
        auto *btn = new QPushButton(text, parent);
        btn->setFixedHeight(48);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(R"(
            QPushButton {
                background-color: #A3C9A3;
                color: #4A3F35;
                border: none;
                border-radius: 14px;
                padding: 10px 28px;
                font-size: 16px;
                font-weight: bold;
                min-width: 180px;
            }
            QPushButton:hover {
                background-color: #B5D5B5;
            }
            QPushButton:pressed {
                background-color: #8FBC8F;
            }
        )");

        // Hover animation — slight scale
        connect(btn, &QPushButton::pressed, btn, [btn]() {
            auto *anim = new QPropertyAnimation(btn, "geometry");
            anim->setDuration(80);
            QRect g = btn->geometry();
            anim->setStartValue(g);
            anim->setEndValue(g.adjusted(-2, -1, 2, 1));
            anim->start(QAbstractAnimation::DeleteWhenStopped);
        });

        return btn;
    };

    QPushButton *newBtn = makeButton("开始新游戏", m_cardWidget);
    QPushButton *saveBtn = makeButton("保存游戏", m_cardWidget);
    QPushButton *loadBtn = makeButton("读取存档", m_cardWidget);
    QPushButton *settingsBtn = makeButton("设置", m_cardWidget);
    QPushButton *helpBtn = makeButton("帮助", m_cardWidget);
    QPushButton *quitBtn = makeButton("退出游戏", m_cardWidget);

    quitBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #E8C86A;
            color: #4A3F35;
            border: none;
            border-radius: 14px;
            padding: 10px 28px;
            font-size: 16px;
            font-weight: bold;
            min-width: 180px;
        }
        QPushButton:hover {
            background-color: #F0D88A;
        }
        QPushButton:pressed {
            background-color: #D4B050;
        }
    )");

    connect(newBtn, &QPushButton::clicked, this, &MenuScreen::newGameClicked);
    connect(saveBtn, &QPushButton::clicked, this, &MenuScreen::saveGameClicked);
    connect(loadBtn, &QPushButton::clicked, this, &MenuScreen::loadGameClicked);
    connect(settingsBtn, &QPushButton::clicked, this, &MenuScreen::settingsClicked);
    connect(helpBtn, &QPushButton::clicked, this, &MenuScreen::helpClicked);
    connect(quitBtn, &QPushButton::clicked, this, &MenuScreen::quitClicked);

    cardLayout->addWidget(newBtn);
    cardLayout->addWidget(saveBtn);
    cardLayout->addWidget(loadBtn);
    cardLayout->addWidget(settingsBtn);
    cardLayout->addWidget(helpBtn);
    cardLayout->addWidget(quitBtn);

    // Cloud animation timer
    m_cloudTimer = new QTimer(this);
    connect(m_cloudTimer, &QTimer::timeout, this, [this]() {
        for (auto &cloud : m_clouds) {
            QRect geo = cloud.label->geometry();
            int newX = geo.x() - static_cast<int>(cloud.speed);
            if (newX + geo.width() < 0) {
                newX = width() + 20;
                cloud.label->setGeometry(newX, static_cast<int>(cloud.yBase),
                                         geo.width(), geo.height());
            } else {
                // Sinusoidal vertical wobble
                double t = QDateTime::currentMSecsSinceEpoch() / 1000.0;
                double wobble = std::sin(t * 0.8 + cloud.yBase) * cloud.amplitude;
                cloud.label->move(newX, static_cast<int>(cloud.yBase + wobble));
            }
        }
    });
}

void MenuScreen::initClouds()
{
    // Remove old clouds
    for (auto &c : m_clouds)
        delete c.label;
    m_clouds.clear();

    // Create 6 clouds at random positions
    for (int i = 0; i < 6; ++i) {
        auto *cloudLabel = new QLabel(this);
        cloudLabel->setStyleSheet(R"(
            background: rgba(255, 255, 255, 0.7);
            border-radius: 30px;
        )");

        int w = 100 + QRandomGenerator::global()->bounded(120);
        int h = 40 + QRandomGenerator::global()->bounded(30);
        int x = QRandomGenerator::global()->bounded(width());
        int y = 30 + QRandomGenerator::global()->bounded(height() / 3);
        cloudLabel->setGeometry(x, y, w, h);
        cloudLabel->show();

        Cloud cloud;
        cloud.label = cloudLabel;
        cloud.speed = 0.3 + QRandomGenerator::global()->bounded(100) / 200.0; // 0.3–0.8 px/tick
        cloud.yBase = y;
        cloud.amplitude = 3 + QRandomGenerator::global()->bounded(10);
        m_clouds.append(cloud);
    }
}

void MenuScreen::startCloudAnimations()
{
    if (m_clouds.isEmpty())
        initClouds();
    m_cloudTimer->start(40); // ~25 fps
}

void MenuScreen::stopCloudAnimations()
{
    m_cloudTimer->stop();
}

void MenuScreen::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    // Light blue sky gradient
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0.0, QColor("#C4DFE6"));
    gradient.setColorAt(0.5, QColor("#D4EAF0"));
    gradient.setColorAt(1.0, QColor("#B8D8C0"));
    painter.fillRect(rect(), gradient);

    // Draw subtle grid dots for texture
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 15));
    for (int x = 0; x < width(); x += 40)
        for (int y = 0; y < height(); y += 40)
            painter.drawEllipse(QPointF(x, y), 1.5, 1.5);
}

void MenuScreen::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    // Re-center card
    if (m_cardWidget) {
        int cx = (width() - m_cardWidget->width()) / 2;
        int cy = (height() - m_cardWidget->sizeHint().height()) / 2;
        m_cardWidget->move(cx, qMax(40, cy));
    }
}

void MenuScreen::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    startCloudAnimations();
}

void MenuScreen::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    stopCloudAnimations();
}
