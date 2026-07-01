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
#include "ui/appviewmodel.h"
#include "audio/audiomanager.h"

namespace {
bool hasSmokeTestArg(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i) {
        if (QString::fromLocal8Bit(argv[i]) == QStringLiteral("--smoke-test")) {
            return true;
        }
    }
    return false;
}

bool hasCalibrateArg(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i) {
        if (QString::fromLocal8Bit(argv[i]) == QStringLiteral("--calibrate")) {
            return true;
        }
    }
    return false;
}

bool runSmokeTestCase(GameEngine &engine,
                      int levelId,
                      const QString &script,
                      int maxSteps,
                      QString &failure,
                      int &ticksUsedOut) {
    bool cleared = false;
    bool failed = false;
    bool errored = false;
    bool scriptDone = false;
    QString failReason;
    QString errorMessage;
    int errorLine = 0;

    const QMetaObject::Connection clearConn = QObject::connect(
        &engine, &GameEngine::levelCleared, &engine, [&](int) { cleared = true; QCoreApplication::instance()->quit(); });
    const QMetaObject::Connection failConn = QObject::connect(
        &engine, &GameEngine::levelFailed, &engine, [&](const QString &reason) {
            failed = true;
            failReason = reason;
            QCoreApplication::instance()->quit();
        });
    const QMetaObject::Connection errorConn = QObject::connect(
        &engine, &GameEngine::errorOccurred, &engine, [&](const QString &msg, int line) {
            errored = true;
            errorMessage = msg;
            errorLine = line;
            QCoreApplication::instance()->quit();
        });
    const QMetaObject::Connection scriptDoneConn = QObject::connect(
        &engine, &GameEngine::scriptCompletedWithoutGoals, &engine, [&]() {
            scriptDone = true;
            QCoreApplication::instance()->quit();
        });
    engine.loadLevel(levelId);
    engine.loadScript(script);
    engine.setSpeed(5.0f);  // max speed for smoke test
    engine.run();
    if (engine.state() != GameEngine::Running) {
        failure = QStringLiteral("level %1 engine did not start (state=%2)")
                      .arg(levelId).arg(static_cast<int>(engine.state()));
        QObject::disconnect(clearConn);
        QObject::disconnect(failConn);
        QObject::disconnect(errorConn);
        QObject::disconnect(scriptDoneConn);
        return false;
    }

    // Use event-loop-based execution. We spin the real event loop so QTimer
    // ticks fire properly, and quit via a one-shot timeout or when the level
    // completes/fails.
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    timeoutTimer.setInterval(60000);  // 60s per level at 5x speed
    timeoutTimer.start();
    const QMetaObject::Connection timeoutConn = QObject::connect(
        &timeoutTimer, &QTimer::timeout, &engine, [&]() {
            if (!cleared && !failed && !errored && !scriptDone) {
                failed = true;
                failReason = QStringLiteral("timeout");
            }
            QCoreApplication::instance()->quit();
        });

    QCoreApplication::exec();

    engine.pause();
    ticksUsedOut = engine.actionTickCount();

    QObject::disconnect(clearConn);
    QObject::disconnect(failConn);
    QObject::disconnect(errorConn);
    QObject::disconnect(scriptDoneConn);
    QObject::disconnect(timeoutConn);

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
    } else if (scriptDone) {
        failure = QStringLiteral("level %1 script completed but goals not met").arg(levelId);
    } else {
        failure = QStringLiteral("level %1 did not finish (event loop exited unexpectedly)")
                      .arg(levelId);
    }
    return false;
}

int runCalibration(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    app.setApplicationName("Code Farm Calibration");

    GameEngine engine;
    LevelManager levelManager;
    engine.setLevelManager(&levelManager);

    std::cerr << "Calibration: running all 20 levels at 5x speed (bugs forced to 0)\n\n";

    for (int id = 1; id <= 20; ++id) {
        LevelConfig cfg = levelManager.getLevelConfig(id);
        if (cfg.levelId != id) {
            std::cerr << "FAIL: level " << id << " config not found\n";
            return 1;
        }
        // Load level, override bugs to 0 for deterministic tick counting
        engine.loadLevel(id);
        engine.overrideBugProbability(0.0f);
        engine.loadScript(cfg.tutorialCode);

        int ticks = 0;
        QString failure;
        if (!runSmokeTestCase(engine, id, cfg.tutorialCode, 0, failure, ticks)) {
            std::cerr << "  L" << id << " (" << cfg.name.toStdString() << "): FAIL - "
                      << failure.toStdString() << "\n";
            continue;
        }
        std::cerr << "  L" << id << " (" << cfg.name.toStdString()
                  << "): ticks=" << ticks
                  << " ★2<=" << cfg.star2TickThreshold
                  << " ★3<=" << cfg.star3TickThreshold << "\n";
    }
    return 0;
}

int runSmokeTests(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    app.setApplicationName("Code Farm Smoke Test");

    GameEngine engine;
    LevelManager levelManager;
    engine.setLevelManager(&levelManager);

    // Verify all 20 levels load correctly and have valid configs
    for (int id = 1; id <= 20; ++id) {
        LevelConfig cfg = levelManager.getLevelConfig(id);
        if (cfg.levelId != id) {
            std::cerr << "FAIL: level " << id << " config not found\n";
            return 1;
        }
        if (cfg.tutorialCode.isEmpty()) {
            std::cerr << "FAIL: level " << id << " tutorial code is empty\n";
            return 1;
        }
        if (cfg.goals.isEmpty()) {
            std::cerr << "FAIL: level " << id << " no goals defined\n";
            return 1;
        }
    }
    std::cerr << "All 20 level configs validated OK\n";

    // Smoke test: run each level's tutorial code and verify it clears
    // Levels with bugProbability > 0 are non-deterministic (random bug spawns),
    // so we only validate their configs and skip the tutorial execution test.
    int passed = 0;
    int skipped = 0;
    for (int id = 1; id <= 20; ++id) {
        LevelConfig cfg = levelManager.getLevelConfig(id);
        if (cfg.bugProbability > 0.0f) {
            skipped++;
            std::cerr << "  SKIP: level " << id << " (" << cfg.name.toStdString()
                      << ") - has random bugs\n";
            continue;
        }
        QString failure;
        int ticksUnused = 0;
        if (!runSmokeTestCase(engine, id, cfg.tutorialCode, 500000, failure, ticksUnused)) {
            std::cerr << "FAIL: level " << id << " (" << cfg.name.toStdString()
                      << "): " << failure.toStdString() << '\n';
            return 1;
        }
        passed++;
        std::cerr << "  PASS: level " << id << " (" << cfg.name.toStdString() << ")\n";
    }

    std::cerr << "\n" << passed << " levels passed, " << skipped
              << " skipped (random bugs)\n";
    return 0;
}
} // namespace

int main(int argc, char *argv[]) {
    if (hasSmokeTestArg(argc, argv)) {
        return runSmokeTests(argc, argv);
    }
    if (hasCalibrateArg(argc, argv)) {
        return runCalibration(argc, argv);
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
    QFontDatabase::addApplicationFont(":/CodeFarm/resources/fonts/fredoka-one.one-regular.ttf");

    QQmlApplicationEngine engine;
    qmlRegisterType<SyntaxHighlighter>("CodeFarm", 1, 0, "SyntaxHighlighter");

    auto *gameEngine = new GameEngine(&app);
    auto *levelManager = new LevelManager(&app);
    auto *saveManager = new SaveManager(&app);
    auto *audioManager = new AudioManager(&app);

    saveManager->load();
    levelManager->loadProgress(saveManager->levelProgress());
    gameEngine->setLevelManager(levelManager);
    auto *appVm = new AppViewModel(gameEngine, levelManager, saveManager, &app);
    gameEngine->setSpeed(appVm->runSpeed());
    audioManager->startBgm();

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
    engine.rootContext()->setContextProperty("appVm", appVm);
    engine.rootContext()->setContextProperty("audioManager", audioManager);

    const QUrl url(QStringLiteral("qrc:/CodeFarm/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
