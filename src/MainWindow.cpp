#include "MainWindow.h"

#include <QFile>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QScrollArea>
#include <QScrollBar>
#include <QTime>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

namespace {

QString setupLabel(const QString &setup)
{
    if (setup == "tilled") return "预开垦";
    if (setup == "dry") return "干旱";
    if (setup == "bugs") return "高虫害";
    return "标准";
}

QString formatGoals(const QVector<Goal> &goals, bool multiline)
{
    QStringList parts;
    for (const auto &goal : goals) {
        const CropInfo *info = cropInfo(goal.crop);
        const QString cropName = info ? info->name : goal.crop;
        const QString icon = info ? info->icon : goal.crop.left(1);
        parts << QString("%1 %2 × %3").arg(icon, cropName).arg(goal.target);
    }
    return parts.join(multiline ? "\n" : "   ");
}

QString formatFunctions(const QStringList &functions)
{
    QStringList calls;
    for (const auto &fn : functions) calls << fn + "()";
    return calls.join("  ·  ");
}

QString formatSyntax(const QStringList &syntax)
{
    return syntax.join("  ·  ");
}

} // namespace

MainWindow::MainWindow()
{
    levels = buildLevels();
    engine = new GameEngine(this);
    settings = new QSettings("CodeFarmQt", "CodeFarm", this);
    buildUi();
    loadProgress();
    refreshOverviewLabels();
    showHome();

    connect(engine, &GameEngine::logMessage, this, &MainWindow::appendLog);
    connect(engine, &GameEngine::stateChanged, this, &MainWindow::updatePanels);
    connect(engine, &GameEngine::commandHighlight, this, [this](int index, const QString &line) {
        if (!editor || !commandLabel) return;
        commandLabel->setText(QString("tick %1  |  %2").arg(index + 1, 2, 10, QChar('0')).arg(line));
        QList<QTextEdit::ExtraSelection> selections;
        QTextEdit::ExtraSelection sel;
        sel.format.setBackground(QColor(255, 205, 117, 95));
        sel.format.setProperty(QTextFormat::FullWidthSelection, true);
        QTextCursor cursor(editor->document()->findBlockByLineNumber(index));
        if (cursor.block().isValid()) {
            sel.cursor = cursor;
            sel.cursor.clearSelection();
            selections << sel;
        }
        editor->setExtraSelections(selections);
        if (canvas) canvas->flashAction();
    });
    connect(engine, &GameEngine::levelFinished, this, [this](bool success, int award) {
        if (success && award > stars.value(level.id)) {
            stars[level.id] = award;
            settings->setValue(QString("stars/%1").arg(level.id), award);
        }
        refreshOverviewLabels();
        refreshLevels();
        updatePanels();
        commandLabel->setText(success ? "订单完成" : "订单失败");
        QMessageBox::information(
            this,
            success ? "任务完成" : "任务失败",
            success ? QString("关卡完成，获得 %1 星。").arg(award)
                    : "时间耗尽或目标未完成，请调整脚本后重试。");
    });

    resize(1460, 900);
    setMinimumSize(1240, 780);
    setWindowTitle("Code Farm / 编程农场");
}

void MainWindow::buildUi()
{
    stack = new QStackedWidget;
    setCentralWidget(stack);

    homePage = makeHomePage();
    levelsPage = makeLevelsPage();
    briefPage = makeBriefPage();
    gamePage = makeGamePage();

    stack->addWidget(homePage);
    stack->addWidget(levelsPage);
    stack->addWidget(briefPage);
    stack->addWidget(gamePage);

    QFile style(":/style.qss");
    if (style.open(QFile::ReadOnly | QFile::Text))
        setStyleSheet(QString::fromUtf8(style.readAll()));
}

QWidget *MainWindow::makeHomePage()
{
    auto *page = new QWidget;
    page->setObjectName("HomePage");

    auto *outer = new QVBoxLayout(page);
    outer->setContentsMargins(26, 24, 26, 24);
    outer->setSpacing(18);

    auto *ribbon = new QFrame;
    ribbon->setObjectName("RibbonBar");
    auto *ribbonLayout = new QHBoxLayout(ribbon);
    ribbonLayout->setContentsMargins(18, 14, 18, 14);
    ribbonLayout->setSpacing(12);

    auto *brandBlock = new QVBoxLayout;
    brandBlock->setSpacing(2);
    auto *brand = new QLabel("Code Farm");
    brand->setObjectName("BrandTitle");
    auto *brandMeta = new QLabel("自动化果园控制台");
    brandMeta->setObjectName("Muted");
    brandBlock->addWidget(brand);
    brandBlock->addWidget(brandMeta);

    homeProgressLabel = new QLabel;
    homeProgressLabel->setObjectName("StatusPill");

    ribbonLayout->addLayout(brandBlock);
    ribbonLayout->addStretch();
    ribbonLayout->addWidget(homeProgressLabel, 0, Qt::AlignVCenter);
    outer->addWidget(ribbon);

    auto *mainRow = new QHBoxLayout;
    mainRow->setSpacing(18);

    auto *heroPanel = new QFrame;
    heroPanel->setObjectName("HeroPanel");
    auto *heroLayout = new QVBoxLayout(heroPanel);
    heroLayout->setContentsMargins(34, 34, 34, 34);
    heroLayout->setSpacing(14);
    heroLayout->addStretch(1);

    auto *kicker = new QLabel("DAY SHIFT");
    kicker->setObjectName("PanelKicker");
    auto *title = new QLabel("编程农场");
    title->setObjectName("HeroTitle");
    title->setWordWrap(true);
    auto *subtitle = new QLabel("用脚本调度无人机，在明亮的农田里完成播种、维护和收割。");
    subtitle->setObjectName("HeroSubtitle");
    subtitle->setWordWrap(true);
    auto *desc = new QLabel("界面整体按轻量农场沙盒重做，强调白天场景、软色面板和更清晰的任务流。");
    desc->setObjectName("InfoText");
    desc->setWordWrap(true);
    homeSummaryLabel = new QLabel;
    homeSummaryLabel->setObjectName("InfoText");
    homeSummaryLabel->setWordWrap(true);

    auto *buttonRow = new QHBoxLayout;
    buttonRow->setSpacing(10);
    auto *start = primaryButton("开始种植");
    auto *help = ghostButton("玩法说明");
    auto *reset = ghostButton("重置进度");
    connect(start, &QPushButton::clicked, this, [this] { showLevels(); });
    connect(help, &QPushButton::clicked, this, [this] {
        QMessageBox::information(
            this,
            "玩法说明",
            "在代码窗口输入一行一个命令，例如 till()、plant(\"wheat\")、water()、harvest()。\n"
            "运行后无人机会循环执行脚本，作物随时间成长，成熟且无 Bug 时才能收割。");
    });
    connect(reset, &QPushButton::clicked, this, [this] {
        settings->clear();
        stars.clear();
        refreshOverviewLabels();
        refreshLevels();
        QMessageBox::information(this, "进度已重置", "关卡星级和解锁状态已清空。");
    });
    buttonRow->addWidget(start);
    buttonRow->addWidget(help);
    buttonRow->addWidget(reset);
    buttonRow->addStretch();

    heroLayout->addWidget(kicker);
    heroLayout->addWidget(title);
    heroLayout->addWidget(subtitle);
    heroLayout->addWidget(desc);
    heroLayout->addWidget(homeSummaryLabel);
    heroLayout->addSpacing(6);
    heroLayout->addLayout(buttonRow);
    heroLayout->addStretch(2);

    auto *overviewCard = new QFrame;
    overviewCard->setObjectName("OverviewCard");
    overviewCard->setMinimumWidth(320);
    auto *overviewLayout = new QVBoxLayout(overviewCard);
    overviewLayout->setContentsMargins(24, 24, 24, 24);
    overviewLayout->setSpacing(14);

    auto makeMetric = [](const QString &value, const QString &label, const QString &note) {
        auto *tile = new QFrame;
        tile->setObjectName("MetricTile");
        auto *layout = new QVBoxLayout(tile);
        layout->setContentsMargins(16, 14, 16, 14);
        layout->setSpacing(2);
        auto *valueLabel = new QLabel(value);
        valueLabel->setObjectName("MetricValue");
        auto *labelText = new QLabel(label);
        labelText->setObjectName("MetricLabel");
        auto *noteText = new QLabel(note);
        noteText->setObjectName("Muted");
        noteText->setWordWrap(true);
        layout->addWidget(valueLabel);
        layout->addWidget(labelText);
        layout->addWidget(noteText);
        return tile;
    };

    auto *overviewKicker = new QLabel("FARM OVERVIEW");
    overviewKicker->setObjectName("PanelKicker");
    auto *overviewTitle = new QLabel("今日总览");
    overviewTitle->setObjectName("PanelTitle");
    auto *overviewDesc = new QLabel("关卡路线、作物订单和可用指令会在这里逐步解锁。");
    overviewDesc->setObjectName("InfoText");
    overviewDesc->setWordWrap(true);

    overviewLayout->addWidget(overviewKicker);
    overviewLayout->addWidget(overviewTitle);
    overviewLayout->addWidget(overviewDesc);
    overviewLayout->addWidget(makeMetric("20", "订单关卡", "从单地块小麦到多作物大农场。"));
    overviewLayout->addWidget(makeMetric("10", "可用指令", "移动、开垦、种植、施肥、调试和收割。"));
    overviewLayout->addWidget(makeMetric("Qt 6", "桌面前端", "统一改成浅色系农场控制台视觉。"));
    overviewLayout->addStretch();

    mainRow->addWidget(heroPanel, 3);
    mainRow->addWidget(overviewCard, 2);
    outer->addLayout(mainRow, 1);

    auto *background = new FarmBackground(page);
    outer->addWidget(background);
    return page;
}

QWidget *MainWindow::makeLevelsPage()
{
    auto *page = new QWidget;
    page->setObjectName("LevelsPage");

    auto *root = new QVBoxLayout(page);
    root->setContentsMargins(26, 24, 26, 24);
    root->setSpacing(18);

    auto *topBar = new QFrame;
    topBar->setObjectName("RibbonBar");
    auto *top = new QHBoxLayout(topBar);
    top->setContentsMargins(18, 14, 18, 14);
    top->setSpacing(12);

    auto *back = ghostButton("返回主页");
    connect(back, &QPushButton::clicked, this, [this] { showHome(); });

    auto *titleBlock = new QVBoxLayout;
    titleBlock->setSpacing(2);
    auto *title = new QLabel("关卡路线");
    title->setObjectName("SectionTitle");
    auto *hint = new QLabel("按地图尺寸、可用函数和订单复杂度逐步升级。");
    hint->setObjectName("Muted");
    titleBlock->addWidget(title);
    titleBlock->addWidget(hint);

    levelsProgressLabel = new QLabel;
    levelsProgressLabel->setObjectName("StatusPill");

    top->addWidget(back, 0, Qt::AlignTop);
    top->addLayout(titleBlock);
    top->addStretch();
    top->addWidget(levelsProgressLabel, 0, Qt::AlignVCenter);
    root->addWidget(topBar);

    auto *summaryStrip = new QFrame;
    summaryStrip->setObjectName("OverviewCard");
    auto *summaryLayout = new QHBoxLayout(summaryStrip);
    summaryLayout->setContentsMargins(20, 16, 20, 16);
    summaryLayout->setSpacing(18);
    auto *summaryTitle = new QLabel("调度建议");
    summaryTitle->setObjectName("PanelTitle");
    auto *summaryText = new QLabel("先把路线跑通，再补浇水和 debug 节奏。高价值作物需要更稳的轮询周期。");
    summaryText->setObjectName("InfoText");
    summaryText->setWordWrap(true);
    summaryLayout->addWidget(summaryTitle);
    summaryLayout->addWidget(summaryText, 1);
    root->addWidget(summaryStrip);

    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto *body = new QWidget;
    auto *bodyLayout = new QVBoxLayout(body);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    levelGrid = new QGridLayout;
    levelGrid->setSpacing(16);
    bodyLayout->addLayout(levelGrid);
    bodyLayout->addStretch();
    for (int i = 0; i < 4; ++i) levelGrid->setColumnStretch(i, 1);
    scroll->setWidget(body);
    root->addWidget(scroll, 1);
    return page;
}

QWidget *MainWindow::makeBriefPage()
{
    auto *page = new QWidget;
    page->setObjectName("BriefPage");

    auto *outer = new QVBoxLayout(page);
    outer->setContentsMargins(26, 24, 26, 24);
    outer->setSpacing(18);

    auto *topBar = new QFrame;
    topBar->setObjectName("RibbonBar");
    auto *top = new QHBoxLayout(topBar);
    top->setContentsMargins(18, 14, 18, 14);
    top->setSpacing(12);
    auto *back = ghostButton("返回关卡");
    connect(back, &QPushButton::clicked, this, [this] { showLevels(); });
    auto *title = new QLabel("任务简报");
    title->setObjectName("SectionTitle");
    auto *subtitle = new QLabel("先确认目标、函数和起始脚本，再进入农场。");
    subtitle->setObjectName("Muted");
    auto *titleBlock = new QVBoxLayout;
    titleBlock->setSpacing(2);
    titleBlock->addWidget(title);
    titleBlock->addWidget(subtitle);
    top->addWidget(back, 0, Qt::AlignTop);
    top->addLayout(titleBlock);
    top->addStretch();
    outer->addWidget(topBar);

    auto *content = new QHBoxLayout;
    content->setSpacing(18);

    auto *briefPanel = new QFrame;
    briefPanel->setObjectName("BriefPanel");
    auto *briefLayout = new QVBoxLayout(briefPanel);
    briefLayout->setContentsMargins(28, 26, 28, 26);
    briefLayout->setSpacing(12);

    auto *briefKicker = new QLabel("MISSION BRIEF");
    briefKicker->setObjectName("PanelKicker");
    briefTitle = new QLabel;
    briefTitle->setObjectName("SectionTitle");
    briefTitle->setWordWrap(true);
    briefMetaLabel = new QLabel;
    briefMetaLabel->setObjectName("TagSoft");
    briefMetaLabel->setWordWrap(true);
    briefBody = new QLabel;
    briefBody->setObjectName("InfoText");
    briefBody->setWordWrap(true);

    briefLayout->addWidget(briefKicker);
    briefLayout->addWidget(briefTitle);
    briefLayout->addWidget(briefMetaLabel);
    briefLayout->addSpacing(4);
    briefLayout->addWidget(briefBody);
    briefLayout->addStretch();

    auto *guidePanel = new QFrame;
    guidePanel->setObjectName("BriefPanel");
    auto *guideLayout = new QVBoxLayout(guidePanel);
    guideLayout->setContentsMargins(24, 24, 24, 24);
    guideLayout->setSpacing(12);

    auto *apiTitle = new QLabel("可用指令");
    apiTitle->setObjectName("PanelTitle");
    briefApiLabel = new QLabel;
    briefApiLabel->setObjectName("InfoText");
    briefApiLabel->setWordWrap(true);

    auto *syntaxTitle = new QLabel("重点语法");
    syntaxTitle->setObjectName("PanelTitle");
    briefSyntaxLabel = new QLabel;
    briefSyntaxLabel->setObjectName("InfoText");
    briefSyntaxLabel->setWordWrap(true);

    auto *starterTitle = new QLabel("起始脚本");
    starterTitle->setObjectName("PanelTitle");
    briefStarterView = new QTextEdit;
    briefStarterView->setObjectName("InsetText");
    briefStarterView->setReadOnly(true);
    briefStarterView->setFont(QFont("JetBrains Mono", 11));
    briefStarterView->setMinimumHeight(220);

    guideLayout->addWidget(apiTitle);
    guideLayout->addWidget(briefApiLabel);
    guideLayout->addSpacing(4);
    guideLayout->addWidget(syntaxTitle);
    guideLayout->addWidget(briefSyntaxLabel);
    guideLayout->addSpacing(4);
    guideLayout->addWidget(starterTitle);
    guideLayout->addWidget(briefStarterView, 1);

    content->addWidget(briefPanel, 3);
    content->addWidget(guidePanel, 2);
    outer->addLayout(content, 1);

    auto *actionBar = new QFrame;
    actionBar->setObjectName("StatusBand");
    auto *actions = new QHBoxLayout(actionBar);
    actions->setContentsMargins(18, 14, 18, 14);
    auto *backBtn = ghostButton("再看关卡");
    auto *startBtn = primaryButton("载入农场");
    connect(backBtn, &QPushButton::clicked, this, [this] { showLevels(); });
    connect(startBtn, &QPushButton::clicked, this, [this] { startLevel(level.id); });
    actions->addWidget(backBtn);
    actions->addStretch();
    actions->addWidget(startBtn);
    outer->addWidget(actionBar);

    return page;
}

QWidget *MainWindow::makeGamePage()
{
    auto *page = new QWidget;
    page->setObjectName("GamePage");

    auto *root = new QVBoxLayout(page);
    root->setContentsMargins(20, 18, 20, 18);
    root->setSpacing(14);

    auto *toolbar = new QFrame;
    toolbar->setObjectName("RibbonBar");
    auto *bar = new QHBoxLayout(toolbar);
    bar->setContentsMargins(18, 14, 18, 14);
    bar->setSpacing(12);

    auto *back = ghostButton("返回关卡");
    connect(back, &QPushButton::clicked, this, [this] {
        engine->stop();
        showLevels();
    });

    auto *statusBlock = new QVBoxLayout;
    statusBlock->setSpacing(2);
    statusLabel = new QLabel;
    statusLabel->setObjectName("ToolbarTitle");
    statusMetaLabel = new QLabel;
    statusMetaLabel->setObjectName("Muted");
    statusBlock->addWidget(statusLabel);
    statusBlock->addWidget(statusMetaLabel);

    timeBar = new QProgressBar;
    timeBar->setRange(0, 1000);
    timeBar->setTextVisible(false);
    timeBar->setFixedWidth(160);
    timeLabel = new QLabel;
    timeLabel->setObjectName("StatusPill");

    auto *stepBtn = ghostButton("单步");
    auto *pauseBtn = ghostButton("暂停");
    runButton = primaryButton("运行脚本");

    connect(runButton, &QPushButton::clicked, this, [this] {
        engine->run(editor->toPlainText());
        commandLabel->setText("准备执行...");
        updatePanels();
    });
    connect(stepBtn, &QPushButton::clicked, this, [this] {
        engine->step(editor->toPlainText());
        commandLabel->setText("单步执行");
        updatePanels();
    });
    connect(pauseBtn, &QPushButton::clicked, this, [this] {
        engine->stop();
        commandLabel->setText("已暂停");
        updatePanels();
    });

    bar->addWidget(back, 0, Qt::AlignTop);
    bar->addLayout(statusBlock);
    bar->addStretch();
    bar->addWidget(timeBar, 0, Qt::AlignVCenter);
    bar->addWidget(timeLabel, 0, Qt::AlignVCenter);
    bar->addSpacing(6);
    bar->addWidget(stepBtn);
    bar->addWidget(pauseBtn);
    bar->addWidget(runButton);
    root->addWidget(toolbar);

    auto *sidebar = new QFrame;
    sidebar->setObjectName("SidebarPanel");
    sidebar->setMinimumWidth(280);
    auto *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(22, 22, 22, 22);
    sidebarLayout->setSpacing(10);

    auto *goalTitle = new QLabel("订单目标");
    goalTitle->setObjectName("PanelTitle");
    goalLabel = new QLabel;
    goalLabel->setObjectName("GoalText");
    goalLabel->setFont(QFont("JetBrains Mono", 11));
    goalLabel->setWordWrap(true);

    auto *apiTitle = new QLabel("可用指令");
    apiTitle->setObjectName("PanelTitle");
    apiLabel = new QLabel;
    apiLabel->setObjectName("InfoText");
    apiLabel->setWordWrap(true);

    auto *syntaxTitle = new QLabel("重点语法");
    syntaxTitle->setObjectName("PanelTitle");
    syntaxLabel = new QLabel;
    syntaxLabel->setObjectName("InfoText");
    syntaxLabel->setWordWrap(true);

    auto *telemetryTitle = new QLabel("运行信息");
    telemetryTitle->setObjectName("PanelTitle");
    telemetryLabel = new QLabel;
    telemetryLabel->setObjectName("InfoText");
    telemetryLabel->setWordWrap(true);

    sidebarLayout->addWidget(goalTitle);
    sidebarLayout->addWidget(goalLabel);
    sidebarLayout->addSpacing(6);
    sidebarLayout->addWidget(apiTitle);
    sidebarLayout->addWidget(apiLabel);
    sidebarLayout->addSpacing(6);
    sidebarLayout->addWidget(syntaxTitle);
    sidebarLayout->addWidget(syntaxLabel);
    sidebarLayout->addSpacing(6);
    sidebarLayout->addWidget(telemetryTitle);
    sidebarLayout->addWidget(telemetryLabel);
    sidebarLayout->addStretch();

    auto *canvasPanel = new QFrame;
    canvasPanel->setObjectName("CanvasPanel");
    auto *canvasLayout = new QVBoxLayout(canvasPanel);
    canvasLayout->setContentsMargins(18, 18, 18, 18);
    canvasLayout->setSpacing(12);

    auto *canvasHeader = new QHBoxLayout;
    auto *canvasTitle = new QLabel("农场视图");
    canvasTitle->setObjectName("PanelTitle");
    auto *canvasHint = new QLabel("滚轮缩放  ·  右键拖动");
    canvasHint->setObjectName("Tag");
    canvasHeader->addWidget(canvasTitle);
    canvasHeader->addStretch();
    canvasHeader->addWidget(canvasHint);

    canvas = new FarmCanvas;
    canvasLayout->addLayout(canvasHeader);
    canvasLayout->addWidget(canvas, 1);

    auto *codePanel = new QFrame;
    codePanel->setObjectName("StudioPanel");
    auto *codeLayout = new QVBoxLayout(codePanel);
    codeLayout->setContentsMargins(18, 18, 18, 18);
    codeLayout->setSpacing(10);
    auto *codeTitle = new QLabel("调度脚本");
    codeTitle->setObjectName("PanelTitle");
    editor = new CodeEditor;
    editor->setObjectName("CodeEditor");
    editor->setFont(QFont("JetBrains Mono", 12));
    editor->setPlaceholderText("一行一个命令，例如:\nplant(\"wheat\")\nwater()");
    new CodeHighlighter(editor->document());
    codeLayout->addWidget(codeTitle);
    codeLayout->addWidget(editor, 1);

    auto *logPanel = new QFrame;
    logPanel->setObjectName("StudioPanel");
    auto *logLayout = new QVBoxLayout(logPanel);
    logLayout->setContentsMargins(18, 18, 18, 18);
    logLayout->setSpacing(10);
    auto *logTitle = new QLabel("终端输出");
    logTitle->setObjectName("PanelTitle");
    logView = new QTextEdit;
    logView->setObjectName("TerminalView");
    logView->setReadOnly(true);
    logView->setFont(QFont("JetBrains Mono", 11));
    logView->document()->setMaximumBlockCount(400);
    logLayout->addWidget(logTitle);
    logLayout->addWidget(logView, 1);

    studioSplitter = new QSplitter(Qt::Vertical);
    studioSplitter->addWidget(codePanel);
    studioSplitter->addWidget(logPanel);
    studioSplitter->setStretchFactor(0, 3);
    studioSplitter->setStretchFactor(1, 2);
    studioSplitter->setSizes({520, 240});

    workspaceSplitter = new QSplitter(Qt::Horizontal);
    workspaceSplitter->addWidget(sidebar);
    workspaceSplitter->addWidget(canvasPanel);
    workspaceSplitter->addWidget(studioSplitter);
    workspaceSplitter->setStretchFactor(0, 0);
    workspaceSplitter->setStretchFactor(1, 1);
    workspaceSplitter->setStretchFactor(2, 1);
    workspaceSplitter->setSizes({290, 620, 430});
    root->addWidget(workspaceSplitter, 1);

    auto *statusBar = new QFrame;
    statusBar->setObjectName("StatusBand");
    auto *sbLayout = new QHBoxLayout(statusBar);
    sbLayout->setContentsMargins(18, 12, 18, 12);
    sbLayout->setSpacing(12);
    commandLabel = new QLabel("等待运行");
    commandLabel->setObjectName("CommandLabel");
    posLabel = new QLabel("无人机: (0, 0)");
    posLabel->setObjectName("Muted");
    auto *verLabel = new QLabel("Code Farm v2 UI");
    verLabel->setObjectName("Muted");
    sbLayout->addWidget(commandLabel, 1);
    sbLayout->addWidget(posLabel);
    sbLayout->addWidget(verLabel);
    root->addWidget(statusBar);

    return page;
}

QPushButton *MainWindow::primaryButton(const QString &text)
{
    auto *b = new CustomButton(text);
    b->setObjectName("Primary");
    return b;
}

QPushButton *MainWindow::ghostButton(const QString &text)
{
    auto *b = new CustomButton(text);
    b->setObjectName("Ghost");
    return b;
}

void MainWindow::loadProgress()
{
    for (const auto &lv : levels)
        stars[lv.id] = settings->value(QString("stars/%1").arg(lv.id), 0).toInt();
}

bool MainWindow::unlocked(int id) const
{
    return id == 1 || stars.value(id - 1) > 0;
}

void MainWindow::switchPage(QWidget *page)
{
    stack->setCurrentWidget(page);
    auto *effect = new QGraphicsOpacityEffect(page);
    page->setGraphicsEffect(effect);
    auto *anim = new QPropertyAnimation(effect, "opacity", page);
    anim->setDuration(200);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &QPropertyAnimation::finished, page, [page] {
        page->setGraphicsEffect(nullptr);
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::showHome()
{
    refreshOverviewLabels();
    switchPage(homePage);
}

void MainWindow::showLevels()
{
    refreshLevels();
    switchPage(levelsPage);
}

void MainWindow::refreshOverviewLabels()
{
    int cleared = 0;
    int totalStars = 0;
    int unlockedCount = 0;
    int nextLevel = 0;

    for (const auto &lv : levels) {
        const int levelStars = stars.value(lv.id);
        if (levelStars > 0) ++cleared;
        totalStars += levelStars;
        if (unlocked(lv.id)) ++unlockedCount;
        if (nextLevel == 0 && unlocked(lv.id) && levelStars == 0)
            nextLevel = lv.id;
    }

    const QString progressText = QString("已完成 %1/%2  ·  %3 星")
        .arg(cleared)
        .arg(levels.size())
        .arg(totalStars);

    if (homeProgressLabel) homeProgressLabel->setText(progressText);
    if (levelsProgressLabel) levelsProgressLabel->setText(progressText);

    if (homeSummaryLabel) {
        if (cleared == levels.size()) {
            homeSummaryLabel->setText("20 个关卡已全部完成，现在可以回头继续刷满星。");
        } else if (nextLevel > 0) {
            homeSummaryLabel->setText(
                QString("当前已解锁 %1 个地块，第 %2 关是下一张待完成订单。")
                    .arg(unlockedCount)
                    .arg(nextLevel));
        } else {
            homeSummaryLabel->setText(QString("当前已解锁 %1 个地块，继续进入关卡路线。").arg(unlockedCount));
        }
    }
}

void MainWindow::refreshLevels()
{
    refreshOverviewLabels();

    while (auto *item = levelGrid->takeAt(0)) {
        delete item->widget();
        delete item;
    }

    int row = 0;
    int col = 0;
    for (const auto &lv : levels) {
        const bool open = unlocked(lv.id);
        const int starCount = stars.value(lv.id);

        auto *card = new QFrame;
        card->setObjectName("LevelCard");
        card->setProperty("locked", !open);
        card->setProperty("done", starCount > 0);
        card->setMinimumHeight(210);

        auto *layout = new QVBoxLayout(card);
        layout->setContentsMargins(18, 16, 18, 16);
        layout->setSpacing(8);

        auto *head = new QHBoxLayout;
        head->setSpacing(8);
        auto *num = new QLabel(QString("第 %1 关").arg(lv.id, 2, 10, QChar('0')));
        num->setObjectName("Tag");
        QString stateText = !open ? "未解锁" : (starCount > 0 ? QString("%1 星").arg(starCount) : "待挑战");
        auto *state = new QLabel(stateText);
        state->setObjectName(starCount > 0 ? "StatusPill" : "Tag");
        head->addWidget(num);
        head->addStretch();
        head->addWidget(state);

        auto *name = new QLabel(lv.name);
        name->setObjectName("CardTitle");

        auto *theme = new QLabel(lv.theme);
        theme->setObjectName("Muted");
        theme->setWordWrap(true);
        theme->setMaximumHeight(44);

        auto *goalSummary = new QLabel(formatGoals(lv.goals, false));
        goalSummary->setObjectName("InfoText");
        goalSummary->setWordWrap(true);

        auto *syntaxSummary = new QLabel(QString("重点语法：%1").arg(lv.syntax.join(" / ")));
        syntaxSummary->setObjectName("Muted");
        syntaxSummary->setWordWrap(true);

        auto *metaRow = new QHBoxLayout;
        metaRow->setSpacing(6);
        auto *mapPill = new QLabel(QString("%1 × %2").arg(lv.size.width()).arg(lv.size.height()));
        mapPill->setObjectName("Tag");
        auto *timePill = new QLabel(QString("%1 秒").arg(lv.seconds));
        timePill->setObjectName("Tag");
        auto *setupPill = new QLabel(setupLabel(lv.setup));
        setupPill->setObjectName("Tag");
        metaRow->addWidget(mapPill);
        metaRow->addWidget(timePill);
        metaRow->addWidget(setupPill);
        metaRow->addStretch();

        auto *button = open ? primaryButton(starCount > 0 ? "再次挑战" : "进入关卡")
                            : new QPushButton("尚未解锁");
        button->setEnabled(open);
        connect(button, &QPushButton::clicked, this, [this, id = lv.id] { showBrief(id); });

        layout->addLayout(head);
        layout->addWidget(name);
        layout->addWidget(theme);
        layout->addWidget(goalSummary);
        layout->addWidget(syntaxSummary);
        layout->addStretch();
        layout->addLayout(metaRow);
        layout->addWidget(button);

        levelGrid->addWidget(card, row, col);
        if (++col == 4) {
            col = 0;
            ++row;
        }
    }
}

void MainWindow::showBrief(int id)
{
    level = levels[id - 1];
    briefTitle->setText(QString("第 %1 关 · %2").arg(level.id, 2, 10, QChar('0')).arg(level.name));
    briefMetaLabel->setText(
        QString("地图 %1 × %2   ·   限时 %3 秒   ·   地块状态 %4")
            .arg(level.size.width())
            .arg(level.size.height())
            .arg(level.seconds)
            .arg(setupLabel(level.setup)));

    QStringList bodyLines;
    bodyLines << level.theme << "" << "订单目标";
    for (const auto &goal : level.goals) {
        const CropInfo *info = cropInfo(goal.crop);
        bodyLines << QString("  %1 %2 × %3")
                         .arg(info ? info->icon : goal.crop.left(1),
                              info ? info->name : goal.crop)
                         .arg(goal.target);
    }
    briefBody->setText(bodyLines.join('\n'));
    briefApiLabel->setText(formatFunctions(level.functions));
    briefSyntaxLabel->setText(formatSyntax(level.syntax));
    briefStarterView->setPlainText(level.starter);
    switchPage(briefPage);
}

void MainWindow::startLevel(int id)
{
    level = levels[id - 1];
    engine->loadLevel(level);
    editor->setPlainText(level.starter);
    logView->clear();
    appendLog(QString("进入关卡 %1：%2").arg(level.id).arg(level.name));

    canvas->cells = const_cast<QVector<Cell> *>(&engine->getCells());
    canvas->grid = level.size;
    canvas->drone = engine->getDrone();
    canvas->selected = 0;
    canvas->resetView();

    apiLabel->setText(formatFunctions(level.functions));
    syntaxLabel->setText(formatSyntax(level.syntax));
    commandLabel->setText("等待运行");
    updatePanels();
    switchPage(gamePage);
}

void MainWindow::updatePanels()
{
    if (!canvas) return;

    canvas->drone = engine->getDrone();
    canvas->selected = engine->getDrone().y() * engine->getGridSize().width() + engine->getDrone().x();
    canvas->update();

    goalLabel->setText(engine->goalText());

    const int remaining = std::max(0, int(std::ceil(engine->getLevelSeconds() - engine->getElapsed())));
    timeLabel->setText(QString("剩余 %1 秒").arg(remaining));
    statusLabel->setText(QString("第 %1 关 · %2").arg(level.id, 2, 10, QChar('0')).arg(level.name));
    statusMetaLabel->setText(
        QString("%1   ·   %2 × %3 地图   ·   %4")
            .arg(level.theme)
            .arg(level.size.width())
            .arg(level.size.height())
            .arg(setupLabel(level.setup)));

    const QString runtimeState = engine->isRunning() ? "自动执行中" : "待命";
    telemetryLabel->setText(
        QString("运行状态：%1\n无人机坐标：(%2, %3)\n画布操作：滚轮缩放，右键拖动\n当前地块配置：%4")
            .arg(runtimeState)
            .arg(engine->getDrone().x())
            .arg(engine->getDrone().y())
            .arg(setupLabel(level.setup)));

    if (posLabel) {
        posLabel->setText(QString("无人机: (%1, %2)").arg(engine->getDrone().x()).arg(engine->getDrone().y()));
    }

    if (timeBar) {
        const int value = int(std::clamp(
            (engine->getLevelSeconds() - engine->getElapsed()) / std::max(1, engine->getLevelSeconds()),
            0.0,
            1.0) * 1000);
        timeBar->setValue(value);
    }

    if (runButton)
        runButton->setText(engine->isRunning() ? "运行中..." : "运行脚本");
}

void MainWindow::appendLog(const QString &message)
{
    logView->append(
        QString("<span style='color:#8A8B7C'>[%1]</span> <span style='color:#3A4630'>%2</span>")
            .arg(QTime::currentTime().toString("HH:mm:ss"), message.toHtmlEscaped()));
    auto *bar = logView->verticalScrollBar();
    bar->setValue(bar->maximum());
}
