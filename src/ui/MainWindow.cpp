#include "MainWindow.h"
#include "MenuScreen.h"
#include "MapView.h"
#include "InfoPanel.h"
#include "TechPanel.h"
#include "ScriptEditor.h"
#include "ScriptConsole.h"
#include "script/ScriptEngine.h"
#include "ErrorIndicator.h"
#include "FloatingPanel.h"
#include "core/GameState.h"

#include <QStackedWidget>
#include <QMenuBar>
#include <QToolBar>
#include <QToolButton>
#include <QLineEdit>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QInputDialog>
#include <QCloseEvent>
#include <QResizeEvent>
#include <QFileDialog>
#include <QDateTime>
#include <QDir>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_gameState(nullptr)
{
    setWindowTitle("Code Farm — 智能果园");
    resize(1400, 900);
    setMinimumSize(1024, 680);

    m_gameState = new GameState(this);

    // Create script engine and console first (dependencies for editor)
    m_scriptConsole = new ScriptConsole(m_gameState, this);
    m_scriptEngine = new ScriptEngine(m_gameState, m_scriptConsole, this);
    m_errorIndicator = new ErrorIndicator(m_gameState, this);

    setupUI();
    setupDocks();
    setupMenuBar();
    setupToolBar();

    connect(m_scriptEngine, &ScriptEngine::executionError,
            m_errorIndicator, &ErrorIndicator::addError);
    connect(m_gameState, &GameState::mapSizeChanged, m_mapView, &MapView::rebuildScene);
    connect(m_gameState, &GameState::mapSizeChanged, m_infoPanel, &InfoPanel::refresh);
    connect(m_gameState, &GameState::levelChanged, m_infoPanel, &InfoPanel::refresh);

    showMenu();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    m_centralStack = new QStackedWidget(this);

    m_menuScreen = new MenuScreen(this);
    connect(m_menuScreen, &MenuScreen::newGameClicked, this, &MainWindow::startNewGame);
    connect(m_menuScreen, &MenuScreen::saveGameClicked, this, &MainWindow::saveGame);
    connect(m_menuScreen, &MenuScreen::loadGameClicked, this, &MainWindow::loadGame);
    connect(m_menuScreen, &MenuScreen::settingsClicked, this, [this]() {
        QMessageBox::information(this, tr("设置"), tr("当前版本使用清新低饱和主题，后续可在这里扩展音量、动画速度与字体设置。"));
    });
    connect(m_menuScreen, &MenuScreen::helpClicked, this, [this]() {
        QMessageBox::information(this, tr("帮助"), tr("所有农场操作都由无人机执行。通过科技树解锁 plant(), harvest(), move(), water(), for, if 等能力，用 Python 脚本逐步自动化整座果园。"));
    });
    connect(m_menuScreen, &MenuScreen::quitClicked, this, &QWidget::close);

    m_mapView = new MapView(m_gameState, this);

    m_centralStack->addWidget(m_menuScreen);
    m_centralStack->addWidget(m_mapView);
    setCentralWidget(m_centralStack);
}

void MainWindow::setupDocks()
{
    m_infoPanel = new InfoPanel(m_gameState, this);
    m_infoPanel->setParent(this);
    m_infoPanel->setGeometry(width() - 160, 72, 140, 48);
    m_infoPanel->hide();

    m_techPanel = new TechPanel(m_gameState, this);
    m_techPanelFrame = new FloatingPanel(tr("科技进化"), m_techPanel, this);
    m_techPanelFrame->setGeometry(90, 78, width() - 180, height() - 140);
    m_techPanelFrame->hide();
    connect(m_techPanelFrame, &FloatingPanel::closeRequested, this, [this]() {
        m_techPanelFrame->hide();
    });

    m_scriptConsole->hide();
    createScriptPanel("auto_farm.py", "# Code Farm 智能果园\n# init() 会重置田地和无人机，方便反复调试\n\ninit()\nplant(\"strawberry\")\nwater()\nwait(18000)\nharvest()\n");

    // Floating error indicator inside the main window, aligned near the top-right tools.
    m_errorIndicator->setParent(this);
    m_errorIndicator->hide();
    positionErrorIndicator();
}

void MainWindow::setupMenuBar()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("文件"));

    QAction *newAct = fileMenu->addAction(tr("开始新游戏"));
    connect(newAct, &QAction::triggered, this, &MainWindow::startNewGame);

    QAction *saveAct = fileMenu->addAction(tr("保存游戏"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::saveGame);

    QAction *loadAct = fileMenu->addAction(tr("读取存档"));
    connect(loadAct, &QAction::triggered, this, &MainWindow::loadGame);

    fileMenu->addSeparator();

    QAction *quitAct = fileMenu->addAction(tr("退出游戏"));
    connect(quitAct, &QAction::triggered, this, &QWidget::close);
}

void MainWindow::setupToolBar()
{
    auto *toolbar = new QToolBar(tr("快捷操作"), this);
    toolbar->setObjectName("QuickActionsToolBar");
    toolbar->setMovable(true);
    toolbar->setFloatable(true);
    toolbar->setStyleSheet(R"(
        QToolBar { background: #FAF7F0; border: 1px solid #D4C9B5; border-radius: 12px; padding: 6px; spacing: 8px; }
        QToolButton { background: #D8EBCB; color: #4A3F35; border-radius: 10px; padding: 7px 12px; font-weight: bold; }
        QToolButton:hover { background: #E8F3DF; }
    )");

    QAction *addScriptAct = toolbar->addAction(tr("+ 代码窗口"));
    addScriptAct->setToolTip(tr("新建一个可独立运行的 Python 脚本标签"));
    connect(addScriptAct, &QAction::triggered, this, [this]() {
        createScriptPanel(QString("script_%1.py").arg(m_scriptEditors.size() + 1),
                          "# 新的智能果园脚本\n# plant 后不能立刻 harvest，作物成熟前请 wait 或 scan\n\ninit()\nplant(\"strawberry\")\nwater()\nwait(18000)\nharvest()\n");
    });

    QAction *techAct = toolbar->addAction(tr("科技进化"));
    techAct->setToolTip(tr("打开/关闭多叉科技树界面"));
    connect(techAct, &QAction::triggered, this, [this]() {
        if (!m_gameRunning) return;
        m_techPanel->rebuild();
        m_techPanelFrame->setGeometry(90, 78, width() - 180, height() - 140);
        m_techPanelFrame->setVisible(!m_techPanelFrame->isVisible());
        m_techPanelFrame->raise();
    });

    QAction *resetViewAct = toolbar->addAction(tr("重置视角"));
    connect(resetViewAct, &QAction::triggered, this, [this]() {
        if (m_gameRunning) m_mapView->resetViewTransform();
    });

    addToolBar(Qt::TopToolBarArea, toolbar);
}

void MainWindow::showMenu()
{
    m_gameRunning = false;
    m_gameState->stopTicking();
    m_centralStack->setCurrentWidget(m_menuScreen);
    m_infoPanel->hide();
    m_techPanelFrame->hide();
    for (auto *panel : m_scriptPanelFrames)
        panel->hide();
    m_errorIndicator->hide();
}

void MainWindow::startNewGame()
{
    m_gameState->reset();
    enterGame();
}

void MainWindow::enterGame()
{
    m_gameRunning = true;
    m_gameState->startTicking();
    m_centralStack->setCurrentWidget(m_mapView);

    m_infoPanel->show();
    m_techPanelFrame->hide();
    for (auto *panel : m_scriptPanelFrames)
        panel->show();

    m_mapView->rebuildScene();
    m_techPanel->rebuild();
    m_infoPanel->refresh();
    if (!m_gameState->scripts().isEmpty()) {
        for (auto *panel : m_scriptPanelFrames)
            panel->deleteLater();
        m_scriptPanelFrames.clear();
        m_scriptEditors.clear();
        int offset = 0;
        for (const auto &script : m_gameState->scripts()) {
            createScriptPanel(script.name, script.code);
            m_scriptPanelFrames.last()->move(width() - 510 - offset, 130 + offset);
            offset += 28;
        }
    }
    positionHud();
    positionErrorIndicator();
}

void MainWindow::loadGame()
{
    QString path = QFileDialog::getOpenFileName(
        this, tr("读取存档"), QDir::homePath() + "/.codefarm/saves",
        tr("JSON 存档 (*.json)"));
    if (path.isEmpty()) return;

    if (m_gameState->loadFromFile(path)) {
        enterGame();
    } else {
        QMessageBox::warning(this, tr("读取失败"), tr("无法读取存档文件。"));
    }
}

void MainWindow::saveGame()
{
    if (!m_gameRunning) return;
    syncScriptsToState();

    QDir dir(QDir::homePath() + "/.codefarm/saves");
    if (!dir.exists())
        QDir().mkpath(dir.absolutePath());

    QString path = QFileDialog::getSaveFileName(
        this, tr("保存游戏"),
        dir.absolutePath() + "/save_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".json",
        tr("JSON 存档 (*.json)"));
    if (path.isEmpty()) return;

    if (m_gameState->saveToFile(path, {})) {
        QMessageBox::information(this, tr("保存成功"), tr("游戏已保存。"));
    } else {
        QMessageBox::warning(this, tr("保存失败"), tr("无法写入存档文件。"));
    }
}

void MainWindow::createScriptPanel(const QString &name, const QString &code)
{
    auto *editor = new ScriptEditor(m_gameState, m_scriptEngine, m_scriptConsole,
                                   m_errorIndicator, this);
    editor->loadSingleScript(name.isEmpty() ? QString("script_%1.py").arg(m_scriptEditors.size() + 1) : name,
                             code);

    auto *panel = new FloatingPanel(tr("代码窗口 %1").arg(m_scriptEditors.size() + 1), editor, this);
    auto *runButton = panel->addTitleButton(tr("运行"), tr("运行脚本"));
    auto *stopButton = panel->addTitleButton(tr("停止"), tr("停止脚本"));
    auto *renameButton = panel->addTitleButton(tr("重命名"), tr("修改脚本名称"));
    const int offset = m_scriptEditors.size() * 24;
    panel->setGeometry(width() - 520 - offset, 116 + offset, 470, 430);
    panel->setPanelMinimized(false);
    panel->setVisible(m_gameRunning);
    panel->raise();

    connect(editor, &ScriptEditor::titleChanged, this, [panel](ScriptEditor *, const QString &title) {
        panel->setTitle(title);
    });
    connect(runButton, &QToolButton::clicked, editor, &ScriptEditor::runScript);
    connect(stopButton, &QToolButton::clicked, editor, &ScriptEditor::stopScript);
    connect(renameButton, &QToolButton::clicked, this, [this, editor]() {
        bool ok = false;
        const QString name = QInputDialog::getText(this, tr("重命名脚本"), tr("脚本名称"),
                                                   QLineEdit::Normal, editor->currentName(), &ok);
        if (ok)
            editor->renameScript(name);
    });
    connect(panel, &FloatingPanel::closeRequested, this, [this, editor](FloatingPanel *) {
        removeScriptPanel(editor);
    });
    panel->setTitle(editor->currentName());

    m_scriptEditors.append(editor);
    m_scriptPanelFrames.append(panel);
}

void MainWindow::removeScriptPanel(ScriptEditor *editor)
{
    const int idx = m_scriptEditors.indexOf(editor);
    if (idx < 0) return;

    FloatingPanel *panel = m_scriptPanelFrames.value(idx);
    m_scriptEditors.removeAt(idx);
    m_scriptPanelFrames.removeAt(idx);
    if (panel) panel->deleteLater();
}

void MainWindow::syncScriptsToState()
{
    auto &scripts = m_gameState->scripts();
    scripts.clear();
    for (auto *editor : m_scriptEditors)
        editor->appendScripts(scripts);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    positionErrorIndicator();
    positionHud();
}

void MainWindow::positionErrorIndicator()
{
    if (!m_errorIndicator) return;
    m_errorIndicator->move(width() - m_errorIndicator->width() - 24,
                           menuBar()->height() + 16);
    m_errorIndicator->raise();
}

void MainWindow::positionHud()
{
    if (!m_infoPanel || !m_gameRunning) return;
    m_infoPanel->setGeometry(width() - 160, menuBar()->height() + 44, 140, 48);
    m_infoPanel->raise();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_gameRunning) {
        auto answer = QMessageBox::question(
            this, tr("退出"),
            tr("确定要退出吗？未保存的进度将会丢失。"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (answer != QMessageBox::Yes) {
            event->ignore();
            return;
        }
    }
    event->accept();
}
