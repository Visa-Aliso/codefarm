#ifndef LEVELDATA_H
#define LEVELDATA_H

#include <QString>
#include <QList>
#include <QSet>
#include <QVector>
#include "core/cell.h"

enum class GoalType { HarvestCount, TimeLimit, TickLimit, Custom };
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
    QSet<QString> allowedBuiltins;           // 内置函数白名单（教学层逐关解锁）
    int star2TickThreshold = 0;              // ★2 的 tick 上限
    int star3TickThreshold = 0;              // ★3 的 tick 上限
    QSet<QString> star3RequiredFeatures;     // ★3 要求代码中出现的特性（AST 键 / 函数名）
    QString tutorialCode;
    float bugProbability = 0.005f;
};

#endif // LEVELDATA_H
