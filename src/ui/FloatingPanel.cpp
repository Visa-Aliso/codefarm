#include "FloatingPanel.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QToolButton>
#include <QVBoxLayout>

FloatingPanel::FloatingPanel(const QString &title, QWidget *content, QWidget *parent)
    : QWidget(parent)
    , m_title(title)
    , m_content(content)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setMouseTracking(true);
    setMinimumSize(230, 44);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 8, 10, 10);
    layout->setSpacing(8);

    m_titleBar = new QWidget(this);
    m_titleBar->setObjectName("floatingTitleBar");
    m_titleBar->setFixedHeight(30);
    m_titleBar->setCursor(Qt::SizeAllCursor);
    m_titleBar->installEventFilter(this);

    m_titleLayout = new QHBoxLayout(m_titleBar);
    m_titleLayout->setContentsMargins(10, 0, 4, 0);
    m_titleLayout->setSpacing(8);

    m_titleLabel = new QLabel(title, m_titleBar);
    m_titleLabel->setObjectName("floatingTitle");
    m_titleLabel->installEventFilter(this);
    m_titleLayout->addWidget(m_titleLabel, 1);

    m_minimizeButton = new QToolButton(m_titleBar);
    m_minimizeButton->setObjectName("floatingMinimizeButton");
    m_minimizeButton->setText("-");
    m_minimizeButton->setToolTip(tr("最小化/恢复"));
    m_minimizeButton->setFixedSize(24, 24);
    m_titleLayout->addWidget(m_minimizeButton);

    m_closeButton = new QToolButton(m_titleBar);
    m_closeButton->setObjectName("floatingCloseButton");
    m_closeButton->setText("x");
    m_closeButton->setToolTip(tr("关闭"));
    m_closeButton->setFixedSize(24, 24);
    m_titleLayout->addWidget(m_closeButton);

    layout->addWidget(m_titleBar);

    if (m_content) {
        m_content->setParent(this);
        layout->addWidget(m_content, 1);
    }

    setStyleSheet(R"(
        FloatingPanel {
            background: rgba(255, 251, 241, 236);
            border: 1px solid rgba(149, 132, 101, 90);
            border-radius: 18px;
        }
        #floatingTitleBar {
            background: #EEF6E6;
            border-radius: 14px;
        }
        #floatingTitle {
            color: #47523F;
            font-size: 13px;
            font-weight: 700;
            background: transparent;
        }
        #floatingMinimizeButton {
            background: #DCECCF;
            color: #516047;
            border: none;
            border-radius: 10px;
            font-weight: 800;
        }
        #floatingMinimizeButton:hover {
            background: #CADFBA;
        }
        #floatingCloseButton {
            background: #E9B9AA;
            color: #664238;
            border: none;
            border-radius: 10px;
            font-weight: 800;
        }
        #floatingCloseButton:hover {
            background: #F0C9BD;
        }
    )");

    connect(m_minimizeButton, &QToolButton::clicked, this, [this]() {
        setPanelMinimized(!m_minimized);
    });
    connect(m_closeButton, &QToolButton::clicked, this, [this]() {
        emit closeRequested(this);
    });
}

bool FloatingPanel::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_titleBar || watched == m_titleLabel) {
        if (event->type() == QEvent::MouseButtonPress) {
            mousePressEvent(static_cast<QMouseEvent *>(event));
            return true;
        }
        if (event->type() == QEvent::MouseMove) {
            mouseMoveEvent(static_cast<QMouseEvent *>(event));
            return true;
        }
        if (event->type() == QEvent::MouseButtonRelease) {
            mouseReleaseEvent(static_cast<QMouseEvent *>(event));
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

QToolButton *FloatingPanel::addTitleButton(const QString &text, const QString &toolTip)
{
    auto *button = new QToolButton(m_titleBar);
    button->setText(text);
    button->setToolTip(toolTip);
    button->setFixedHeight(24);
    button->setStyleSheet(R"(
        QToolButton { background: #E6EED6; color: #536246; border: none; border-radius: 10px; padding: 0 10px; font-weight: 700; }
        QToolButton:hover { background: #D6E6C5; }
    )");
    m_titleLayout->insertWidget(qMax(1, m_titleLayout->count() - 2), button);
    return button;
}

void FloatingPanel::setCloseButtonVisible(bool visible)
{
    m_closeButton->setVisible(visible);
}

void FloatingPanel::setPanelMinimized(bool minimized)
{
    if (m_minimized == minimized)
        return;

    m_minimized = minimized;
    if (m_minimized) {
        if (height() > 52)
            m_expandedGeometry = geometry();
        applyMinimizedGeometry();
    } else {
        restoreExpandedGeometry();
    }
    update();
}

void FloatingPanel::setTitle(const QString &title)
{
    m_title = title;
    m_titleLabel->setText(title);
}

void FloatingPanel::applyMinimizedGeometry()
{
    if (m_content)
        m_content->hide();
    m_minimizeButton->setText("+");
    setMinimumSize(190, 44);
    resize(qMax(190, width()), 48);
}

void FloatingPanel::restoreExpandedGeometry()
{
    if (m_content)
        m_content->show();
    m_minimizeButton->setText("-");
    setMinimumSize(230, 140);
    if (m_expandedGeometry.isValid())
        setGeometry(m_expandedGeometry);
    else
        resize(qMax(280, width()), 320);
}

bool FloatingPanel::hitResizeHandle(const QPoint &pos) const
{
    return !m_minimized && QRect(width() - 18, height() - 18, 18, 18).contains(pos);
}

void FloatingPanel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    raise();
    m_pressGlobal = event->globalPosition().toPoint();
    m_pressGeometry = geometry();
    const QPoint panelPos = mapFromGlobal(event->globalPosition().toPoint());
    m_resizing = hitResizeHandle(panelPos);
    m_dragging = !m_resizing && (m_minimized || m_titleBar->geometry().contains(panelPos));
    event->accept();
}

void FloatingPanel::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_dragging && !m_resizing) {
        setCursor(hitResizeHandle(event->pos()) ? Qt::SizeFDiagCursor : Qt::ArrowCursor);
        QWidget::mouseMoveEvent(event);
        return;
    }

    const QPoint delta = event->globalPosition().toPoint() - m_pressGlobal;
    if (m_dragging) {
        QRect next = m_pressGeometry.translated(delta);
        if (parentWidget()) {
            next.moveLeft(qBound(0, next.left(), parentWidget()->width() - 80));
            next.moveTop(qBound(0, next.top(), parentWidget()->height() - 44));
        }
        setGeometry(next);
    } else if (m_resizing) {
        resize(qMax(minimumWidth(), m_pressGeometry.width() + delta.x()),
               qMax(minimumHeight(), m_pressGeometry.height() + delta.y()));
    }
    event->accept();
}

void FloatingPanel::mouseReleaseEvent(QMouseEvent *event)
{
    m_dragging = false;
    m_resizing = false;
    setCursor(Qt::ArrowCursor);
    QWidget::mouseReleaseEvent(event);
}

void FloatingPanel::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    if (m_minimized)
        return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(QColor(151, 132, 99, 80), 1));
    painter.drawLine(width() - 16, height() - 6, width() - 6, height() - 16);
    painter.drawLine(width() - 12, height() - 6, width() - 6, height() - 12);
}
