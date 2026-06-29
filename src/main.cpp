#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickStyle>
#include <QFontDatabase>
#include <QDir>
#include <QEventLoop>
#include <QThread>

#include <iostream>

#include "core/gameengine.h"
#include "core/farmmap.h"
#include "editor/syntaxhighlighter.h"
#include "levels/levelmanager.h"
#include "save/savemanager.h"

namespace {
bool hasSmokeTestArg(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i) {
        if (QString::fromLocal8Bit(argv[i]) == QStringLiteral("--smoke-test")) {
            return true;
        }
    }
    return false;
}

bool runSmokeTestCase(GameEngine &engine,
                      int levelId,
                      const QString &script,
                      int maxSteps,
                      QString &failure) {
    bool cleared = false;
    bool failed = false;
    bool errored = false;
    QString failReason;
    QString errorMessage;
    int errorLine = 0;

    const QMetaObject::Connection clearConn = QObject::connect(
        &engine, &GameEngine::levelCleared, &engine, [&](int) { cleared = true; });
    const QMetaObject::Connection failConn = QObject::connect(
        &engine, &GameEngine::levelFailed, &engine, [&](const QString &reason) {
            failed = true;
            failReason = reason;
        });
    const QMetaObject::Connection errorConn = QObject::connect(
        &engine, &GameEngine::errorOccurred, &engine, [&](const QString &msg, int line) {
            errored = true;
            errorMessage = msg;
            errorLine = line;
        });
    engine.loadLevel(levelId);
    engine.loadScript(script);

    for (int step = 0; step < maxSteps && !cleared && !failed && !errored; ++step) {
        engine.stepOnce();
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        QThread::msleep(2);
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    }

    QObject::disconnect(clearConn);
    QObject::disconnect(failConn);
    QObject::disconnect(errorConn);

    if (cleared) {
        return true;
    }

    if (errored) {
        failure = QStringLiteral("level %1 error at line %2: %3")
                      .arg(levelId)
                      .arg(errorLine)
                      .arg(errorMessage);
    } else if (failed) {
        failure = QStringLiteral("level %1 failed: %2").arg(levelId).arg(failReason);
    } else {
        failure = QStringLiteral("level %1 did not finish within %2 steps")
                      .arg(levelId)
                      .arg(maxSteps);
    }
    return false;
}

int runSmokeTests(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    app.setApplicationName("Code Farm Smoke Test");

    GameEngine engine;
    LevelManager levelManager;
    engine.setLevelManager(&levelManager);

    struct SmokeCase {
        int levelId;
        QString name;
        QString script;
        int maxSteps;
    };

    const QList<SmokeCase> cases = {
        {1,
         QStringLiteral("basic_harvest"),
         QStringLiteral("harvest()\n"),
         4},
        {3,
         QStringLiteral("water_then_harvest"),
         QStringLiteral("water()\nharvest()\n"),
         6},
        {4,
         QStringLiteral("loop_harvest"),
         QStringLiteral("for i in range(2):\n    harvest()\n    move(\"right\")\n\nharvest()\n"),
         10},
        {5,
         QStringLiteral("growth_wait_and_clear"),
         QStringLiteral(
             "till()\n"
             "plant(\"wheat\")\n"
             "water()\n"
             "fertilize()\n"
             "move(\"right\")\n"
             "till()\n"
             "plant(\"wheat\")\n"
             "water()\n"
             "fertilize()\n"
             "move(\"down\")\n"
             "till()\n"
             "plant(\"wheat\")\n"
             "water()\n"
             "fertilize()\n"
             "move(\"left\")\n"
             "till()\n"
             "plant(\"wheat\")\n"
             "water()\n"
             "fertilize()\n"
             "for i in range(12):\n"
             "    wait()\n"
             "harvest()\n"
             "move(\"right\")\n"
             "harvest()\n"
             "move(\"up\")\n"
             "harvest()\n"
             "move(\"left\")\n"
             "harvest()\n"),
         60},
    };

    for (const SmokeCase &testCase : cases) {
        QString failure;
        if (!runSmokeTestCase(engine, testCase.levelId, testCase.script, testCase.maxSteps, failure)) {
            std::cerr << testCase.name.toStdString() << ": " << failure.toStdString() << '\n';
            return 1;
        }
    }

    std::cout << "Smoke tests passed\n";
    return 0;
}
} // namespace

int main(int argc, char *argv[]) {
    if (hasSmokeTestArg(argc, argv)) {
        return runSmokeTests(argc, argv);
    }

    QGuiApplication app(argc, argv);
    app.setApplicationName("Code Farm");
    app.setOrganizationName("CodeFarm");
    app.setApplicationVersion("0.1.0");

    QQuickStyle::setStyle("Fusion");

    // Load fonts from resources
    QFontDatabase::addApplicationFont(":/CodeFarm/resources/fonts/Nunito-Regular.ttf");
    QFontDatabase::addApplicationFont(":/CodeFarm/resources/fonts/Nunito-SemiBold.ttf");
    QFontDatabase::addApplicationFont(":/CodeFarm/resources/fonts/Nunito-Bold.ttf");
    QFontDatabase::addApplicationFont(":/CodeFarm/resources/fonts/JetBrainsMono-Regular.ttf");

    QQmlApplicationEngine engine;
    qmlRegisterType<SyntaxHighlighter>("CodeFarm", 1, 0, "SyntaxHighlighter");

    auto *gameEngine = new GameEngine(&app);
    auto *levelManager = new LevelManager(&app);
    auto *saveManager = new SaveManager(&app);

    saveManager->load();
    levelManager->loadProgress(saveManager->levelProgress());
    gameEngine->setLevelManager(levelManager);

    QObject::connect(gameEngine, &GameEngine::levelCleared, &app, [=](int stars) {
        const int levelId = gameEngine->currentLevelId();
        if (levelId <= 0) {
            return;
        }

        levelManager->recordClear(levelId, gameEngine->timeElapsed(), stars);
        saveManager->setLevelProgress(levelId,
                                      levelManager->getStars(levelId),
                                      levelManager->getBestTime(levelId),
                                      levelManager->getClearCount(levelId));
    });

    engine.rootContext()->setContextProperty("gameEngine", gameEngine);
    engine.rootContext()->setContextProperty("levelManager", levelManager);
    engine.rootContext()->setContextProperty("saveManager", saveManager);
    engine.rootContext()->setContextProperty("farmMap", gameEngine->farmMap());

    const QUrl url(QStringLiteral("qrc:/CodeFarm/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
