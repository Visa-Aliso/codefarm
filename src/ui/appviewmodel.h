#ifndef APPVIEWMODEL_H
#define APPVIEWMODEL_H

#include <QObject>
#include <QVariantList>
#include <QVariantMap>

class FarmMap;
class GameEngine;
class LevelManager;
class SaveManager;

class AppViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList levels READ levels NOTIFY levelsChanged)
    Q_PROPERTY(QVariantMap activeLevel READ activeLevel NOTIFY activeLevelChanged)
    Q_PROPERTY(QVariantList activeGoals READ activeGoals NOTIFY activeGoalsChanged)
    Q_PROPERTY(QVariantList allowedApis READ allowedApis NOTIFY activeLevelChanged)
    Q_PROPERTY(QObject* farmMap READ farmMap NOTIFY farmMapChanged)
    Q_PROPERTY(QString scriptText READ scriptText WRITE saveScript NOTIFY scriptTextChanged)
    Q_PROPERTY(int executingLine READ executingLine NOTIFY executingLineChanged)
    Q_PROPERTY(QString consoleLine READ consoleLine NOTIFY consoleLineChanged)
    Q_PROPERTY(int state READ state NOTIFY runtimeChanged)
    Q_PROPERTY(int tickCount READ tickCount NOTIFY runtimeChanged)
    Q_PROPERTY(int timeElapsed READ timeElapsed NOTIFY runtimeChanged)
    Q_PROPERTY(float energy READ energy NOTIFY runtimeChanged)
    Q_PROPERTY(float maxEnergy READ maxEnergy NOTIFY runtimeChanged)
    Q_PROPERTY(int droneX READ droneX NOTIFY runtimeChanged)
    Q_PROPERTY(int droneY READ droneY NOTIFY runtimeChanged)
    Q_PROPERTY(int totalStars READ totalStars NOTIFY levelsChanged)
    Q_PROPERTY(int completedCount READ completedCount NOTIFY levelsChanged)
    Q_PROPERTY(bool particlesEnabled READ particlesEnabled WRITE setParticlesEnabled NOTIFY uiPreferencesChanged)
    Q_PROPERTY(float motionScale READ motionScale WRITE setMotionScale NOTIFY uiPreferencesChanged)
    Q_PROPERTY(int editorFontSize READ editorFontSize WRITE setEditorFontSize NOTIFY uiPreferencesChanged)

public:
    explicit AppViewModel(GameEngine *engine,
                          LevelManager *levelManager,
                          SaveManager *saveManager,
                          QObject *parent = nullptr);

    QVariantList levels() const;
    QVariantMap activeLevel() const;
    QVariantList activeGoals() const;
    QVariantList allowedApis() const;
    QObject *farmMap() const;
    QString scriptText() const { return scriptText_; }
    int executingLine() const { return executingLine_; }
    QString consoleLine() const { return consoleLine_; }
    int state() const;
    int tickCount() const;
    int timeElapsed() const;
    float energy() const;
    float maxEnergy() const;
    int droneX() const;
    int droneY() const;
    int totalStars() const;
    int completedCount() const;
    bool particlesEnabled() const { return particlesEnabled_; }
    float motionScale() const { return motionScale_; }
    int editorFontSize() const { return editorFontSize_; }

    Q_INVOKABLE void openLevel(int levelId);
    Q_INVOKABLE void runOrPause();
    Q_INVOKABLE void stepOnce();
    Q_INVOKABLE void resetLevel();
    Q_INVOKABLE void giveUp();
    Q_INVOKABLE void saveScript(const QString &text);
    Q_INVOKABLE void resetScriptToTutorial();
    Q_INVOKABLE QVariantMap cellAt(int x, int y) const;
    Q_INVOKABLE int nextUnlockedLevel() const;
    Q_INVOKABLE void resetUiPreferences();
    Q_INVOKABLE QVariantMap level(int levelId) const;
    Q_INVOKABLE QString failureHint(const QString &reason) const;
    Q_INVOKABLE void resetAllProgress();

public slots:
    void setParticlesEnabled(bool enabled);
    void setMotionScale(float scale);
    void setEditorFontSize(int size);

signals:
    void levelsChanged();
    void activeLevelChanged();
    void activeGoalsChanged();
    void farmMapChanged();
    void scriptTextChanged();
    void executingLineChanged();
    void consoleLineChanged();
    void runtimeChanged();
    void uiPreferencesChanged();
    void levelCleared(int stars);
    void levelFailed(const QString &reason);
    void scriptCompletedWithoutGoals();
    void errorOccurred(const QString &message, int line);

private:
    void loadPreferences();
    void savePreferences() const;
    void refreshScriptFromStore();
    void setConsoleLine(const QString &line);

    GameEngine *engine_ = nullptr;
    LevelManager *levelManager_ = nullptr;
    SaveManager *saveManager_ = nullptr;
    int activeLevelId_ = -1;
    QString scriptText_;
    int executingLine_ = -1;
    QString consoleLine_ = QStringLiteral("系统就绪，等待任务。");
    bool particlesEnabled_ = true;
    float motionScale_ = 1.0f;
    int editorFontSize_ = 14;
};

#endif // APPVIEWMODEL_H
