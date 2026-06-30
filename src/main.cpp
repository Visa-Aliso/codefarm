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

bool runSmokeTestCase(GameEngine &engine,
                      int levelId,
                      const QString &script,
                      int maxSteps,
                      QString &failure) {
    bool cleared = false;
    bool failed = false;
    bool errored = false;
    bool scriptDone = false;
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
    const QMetaObject::Connection scriptDoneConn = QObject::connect(
        &engine, &GameEngine::scriptCompletedWithoutGoals, &engine, [&]() {
            scriptDone = true;
        });
    engine.loadLevel(levelId);
    engine.loadScript(script);

    for (int step = 0; step < maxSteps && !cleared && !failed && !errored && !scriptDone; ++step) {
        engine.stepOnce();
        QCoreApplication::processEvents();
    }

    QObject::disconnect(clearConn);
    QObject::disconnect(failConn);
    QObject::disconnect(errorConn);
    QObject::disconnect(scriptDoneConn);

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
    std::cout << "All 20 level configs validated OK\n";

    // Smoke test: run each level's tutorial code and verify it clears
    // Levels with bugProbability > 0 are non-deterministic (random bug spawns),
    // so we only validate their configs and skip the tutorial execution test.
    int passed = 0;
    int skipped = 0;
    for (int id = 1; id <= 20; ++id) {
        LevelConfig cfg = levelManager.getLevelConfig(id);
        if (cfg.bugProbability > 0.0f) {
            skipped++;
            std::cout << "  SKIP: level " << id << " (" << cfg.name.toStdString()
                      << ") - has random bugs\n";
            continue;
        }
        QString failure;
        if (!runSmokeTestCase(engine, id, cfg.tutorialCode, 10000, failure)) {
            std::cerr << "FAIL: level " << id << " (" << cfg.name.toStdString()
                      << "): " << failure.toStdString() << '\n';
            return 1;
        }
        passed++;
        std::cout << "  PASS: level " << id << " (" << cfg.name.toStdString() << ")\n";
    }

    std::cout << "\n" << passed << " levels passed, " << skipped
              << " skipped (random bugs)\n";
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
