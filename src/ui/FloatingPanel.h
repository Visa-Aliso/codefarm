#ifndef FLOATINGPANEL_H
#define FLOATINGPANEL_H

#include <QWidget>

class QLabel;
class QHBoxLayout;
class QToolButton;

class FloatingPanel : public QWidget {
    Q_OBJECT
public:
    explicit FloatingPanel(const QString &title, QWidget *content, QWidget *parent = nullptr);

    QString title() const { return m_title; }
    void setTitle(const QString &title);
    QToolButton *addTitleButton(const QString &text, const QString &toolTip = QString());
    void setCloseButtonVisible(bool visible);
    QWidget *contentWidget() const { return m_content; }
    bool isPanelMinimized() const { return m_minimized; }
    void setPanelMinimized(bool minimized);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

signals:
    void closeRequested(FloatingPanel *panel);

private:
    bool hitResizeHandle(const QPoint &pos) const;
    void applyMinimizedGeometry();
    void restoreExpandedGeometry();

    QString m_title;
    QWidget *m_content;
    QWidget *m_titleBar;
    QLabel *m_titleLabel;
    QToolButton *m_minimizeButton;
    QToolButton *m_closeButton;
    QHBoxLayout *m_titleLayout;

    bool m_dragging = false;
    bool m_resizing = false;
    bool m_minimized = false;
    QPoint m_pressGlobal;
    QRect m_pressGeometry;
    QRect m_expandedGeometry;
};

#endif // FLOATINGPANEL_H
