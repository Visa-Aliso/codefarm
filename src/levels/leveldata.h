#ifndef LEVELDATA_H
#define LEVELDATA_H

#include <QString>
#include <QList>
#include <QSet>
#include <QVector>
#include "core/cell.h"

enum class GoalType { HarvestCount, TimeLimit, Custom };
enum class StarTier { Star1 = 1, Star2 = 2, Star3 = 3 };

struct LevelGoal {
    QString description;
    GoalType type = GoalType::HarvestCount;
    QString cropType;
    int targetValue = 0;
    int currentValue = 0;
    bool completed = false;
    StarTier starTier = StarTier::Star1;
};

struct LevelConfig {
    int levelId = 0;
    QString name;
    QString description;
    int gridW = 1;
    int gridH = 1;
    int droneStartX = 0;
    int droneStartY = 0;
    int timeLimitSec = 0;
    int maxTimeSec = 300;
    QList<LevelGoal> goals;
    QVector<Cell> presetCells;
    QSet<QString> allowedFunctions;
    QSet<QString> allowedSyntax;
    QSet<QString> allowedCrops;
    QString tutorialCode;
    float bugProbability = 0.005f;
};

#endif // LEVELDATA_H
