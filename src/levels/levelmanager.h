#ifndef LEVELMANAGER_H
#define LEVELMANAGER_H

#include <QObject>
#include <QMap>
#include <QJsonObject>
#include <QVariantMap>
#include "leveldata.h"

class LevelManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(int totalStars READ totalStars NOTIFY progressChanged)
    Q_PROPERTY(int completedCount READ completedCount NOTIFY progressChanged)

public:
    explicit LevelManager(QObject *parent = nullptr);

    Q_INVOKABLE QVariantMap getLevel(int id) const;
    Q_INVOKABLE bool isUnlocked(int id) const;
    Q_INVOKABLE int getStars(int id) const;
    Q_INVOKABLE int getBestTime(int id) const;
    Q_INVOKABLE int getClearCount(int id) const;
    Q_INVOKABLE int levelCount() const { return levels_.size(); }

    LevelConfig getLevelConfig(int id) const;
    int totalStars() const;
    int completedCount() const;
    void recordClear(int id, int timeUsed, int starsEarned);
    void loadProgress(const QJsonObject &savedProgress);
    void resetProgress();

signals:
    void progressChanged();
    void levelUnlocked(int id);

private:
    void loadLevels();
    QMap<int, LevelConfig> levels_;

    struct Progress {
        bool unlocked = false;
        int stars = 0;
        int bestTime = -1;
        int clearCount = 0;
    };
    QMap<int, Progress> progress_;
};

#endif // LEVELMANAGER_H
