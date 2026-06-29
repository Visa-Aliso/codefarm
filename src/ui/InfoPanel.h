#ifndef INFOPANEL_H
#define INFOPANEL_H

#include <QWidget>
#include <QStringList>

class QLabel;
class QHBoxLayout;
class GameState;

class InfoPanel : public QWidget {
    Q_OBJECT
public:
    explicit InfoPanel(GameState *state, QWidget *parent = nullptr);

    void refresh();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void setupUI();
    QLabel *makeValueLabel(QHBoxLayout *layout);
    GameState *m_gameState;
    QLabel *m_goldLabel;
    QLabel *m_levelLabel;
    QLabel *m_weatherLabel;
    QLabel *m_mapLabel;
    QLabel *m_droneLabel;
};

#endif // INFOPANEL_H
