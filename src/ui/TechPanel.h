#ifndef TECHPANEL_H
#define TECHPANEL_H

#include <QWidget>
#include <QGraphicsView>
#include "core/TechTree.h"

class GameState;
class QGraphicsScene;
class QLabel;
class QPushButton;

class TechPanel : public QWidget {
    Q_OBJECT
public:
    explicit TechPanel(GameState *state, QWidget *parent = nullptr);

    void rebuild();

private:
    void setupUI();
    void selectNode(const QString &id);
    void refreshDetails();
    const TechNode *nodeById(const QString &id) const;

    GameState *m_gameState;
    QGraphicsView *m_view;
    QGraphicsScene *m_scene;
    QLabel *m_detailTitle;
    QLabel *m_detailBody;
    QPushButton *m_upgradeButton;
    QString m_selectedNodeId;
};

#endif // TECHPANEL_H
