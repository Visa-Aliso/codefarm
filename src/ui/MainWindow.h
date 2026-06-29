#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QVector>

class QResizeEvent;

class MenuScreen;
class MapView;
class InfoPanel;
class TechPanel;
class ScriptEditor;
class ScriptConsole;
class ScriptEngine;
class ErrorIndicator;
class FloatingPanel;
class GameState;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void showMenu();
    void startNewGame();
    void loadGame();
    void saveGame();
    GameState *gameState() const { return m_gameState; }

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupUI();
    void setupDocks();
    void setupMenuBar();
    void setupToolBar();
    void enterGame();
    void positionErrorIndicator();
    void positionHud();
    void createScriptPanel(const QString &name = QString(), const QString &code = QString());
    void removeScriptPanel(ScriptEditor *editor);
    void syncScriptsToState();

    QStackedWidget *m_centralStack;
    MenuScreen *m_menuScreen;
    MapView *m_mapView;

    InfoPanel *m_infoPanel;
    TechPanel *m_techPanel;
    FloatingPanel *m_techPanelFrame;
    QVector<FloatingPanel *> m_scriptPanelFrames;
    QVector<ScriptEditor *> m_scriptEditors;
    ScriptConsole *m_scriptConsole;
    ScriptEngine *m_scriptEngine;
    ErrorIndicator *m_errorIndicator;

    GameState *m_gameState;
    bool m_gameRunning = false;
};

#endif // MAINWINDOW_H
