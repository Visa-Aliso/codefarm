#pragma once

#include "CodeEditor.h"
#include "CodeHighlighter.h"
#include "CustomButton.h"
#include "FarmBackground.h"
#include "FarmCanvas.h"
#include "GameEngine.h"
#include "levels.h"
#include "types.h"

#include <QGridLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMap>
#include <QProgressBar>
#include <QPushButton>
#include <QSettings>
#include <QSplitter>
#include <QStackedWidget>
#include <QTextEdit>

class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    MainWindow();

private:
    QVector<Level> levels;
    Level level;
    GameEngine *engine = nullptr;
    QSettings *settings = nullptr;

    QStackedWidget *stack = nullptr;
    QWidget *homePage = nullptr;
    QWidget *levelsPage = nullptr;
    QWidget *briefPage = nullptr;
    QWidget *gamePage = nullptr;
    QGridLayout *levelGrid = nullptr;

    QLabel *homeProgressLabel = nullptr;
    QLabel *homeSummaryLabel = nullptr;
    QLabel *levelsProgressLabel = nullptr;

    QLabel *briefTitle = nullptr;
    QLabel *briefMetaLabel = nullptr;
    QLabel *briefBody = nullptr;
    QLabel *briefApiLabel = nullptr;
    QLabel *briefSyntaxLabel = nullptr;
    QTextEdit *briefStarterView = nullptr;

    FarmCanvas *canvas = nullptr;
    CodeEditor *editor = nullptr;
    QTextEdit *logView = nullptr;
    QLabel *goalLabel = nullptr;
    QLabel *apiLabel = nullptr;
    QLabel *syntaxLabel = nullptr;
    QLabel *timeLabel = nullptr;
    QLabel *statusLabel = nullptr;
    QLabel *statusMetaLabel = nullptr;
    QLabel *telemetryLabel = nullptr;
    QPushButton *runButton = nullptr;
    QLabel *commandLabel = nullptr;
    QProgressBar *timeBar = nullptr;

    QSplitter *workspaceSplitter = nullptr;
    QSplitter *studioSplitter = nullptr;
    QLabel *posLabel = nullptr;

    QMap<int, int> stars;

    void buildUi();
    QWidget *makeHomePage();
    QWidget *makeLevelsPage();
    QWidget *makeBriefPage();
    QWidget *makeGamePage();

    QPushButton *primaryButton(const QString &text);
    QPushButton *ghostButton(const QString &text);

    void loadProgress();
    bool unlocked(int id) const;
    void switchPage(QWidget *page);
    void showHome();
    void showLevels();
    void refreshLevels();
    void refreshOverviewLabels();
    void showBrief(int id);
    void startLevel(int id);
    void updatePanels();
    void appendLog(const QString &message);
};
