#ifndef MENUSCREEN_H
#define MENUSCREEN_H

#include <QWidget>
#include <QVector>

class QLabel;
class QPushButton;
class QPropertyAnimation;

struct Cloud {
    QLabel *label;
    QPropertyAnimation *anim;
    double speed;
    double yBase;
    double amplitude;
};

class MenuScreen : public QWidget {
    Q_OBJECT
public:
    explicit MenuScreen(QWidget *parent = nullptr);
    ~MenuScreen();

signals:
    void newGameClicked();
    void saveGameClicked();
    void loadGameClicked();
    void settingsClicked();
    void helpClicked();
    void quitClicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    void setupUI();
    void initClouds();
    void startCloudAnimations();
    void stopCloudAnimations();

    QWidget *m_cardWidget;
    QVector<Cloud> m_clouds;
    QTimer *m_cloudTimer;
};

#endif // MENUSCREEN_H
