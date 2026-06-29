#include "levelmanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

LevelManager::LevelManager(QObject *parent) : QObject(parent) {
    loadLevels();
    progress_[1].unlocked = true;
}

void LevelManager::loadLevels() {
    // === 关卡 1：初次收割 (1×1, 小麦80%) ===
    {
        LevelConfig l;
        l.levelId = 1;
        l.name = u"初次收割"_qs;
        l.description = u"认识无人机，学会收割"_qs;
        l.gridW = 1; l.gridH = 1;
        l.maxTimeSec = 300;
        l.droneMaxEnergy = 20.0f;
        l.bugProbability = 0.0f;
        l.allowedFunctions = {"harvest","get_pos","get_map_size"};
        l.allowedSyntax = {"expr","call"};
        l.allowedCrops = {"wheat"};
        l.tutorialCode = u"# 当前格的小麦已经成熟，直接收割\n\nharvest()\n"_qs;
        Cell c; c.x=0; c.y=0; c.state=CellState::Mature;
        c.crop=CropType::Wheat; c.progress=1.0f; c.water=0.5f;
        l.presetCells = {c};
        l.goals = {
            {u"收获小麦 × 1"_qs, GoalType::HarvestCount, "wheat", 1, 0, false, StarTier::Star1},
            {u"60 秒内完成"_qs, GoalType::TimeLimit, "", 60, 0, false, StarTier::Star2},
            {u"10 秒内完成"_qs, GoalType::TimeLimit, "", 10, 0, false, StarTier::Star3}
        };
        levels_[1] = l;
    }

    // === 关卡 2：移动与收割 (3×1, 三格小麦) ===
    {
        LevelConfig l;
        l.levelId = 2;
        l.name = u"移动与收割"_qs;
        l.description = u"学会 move() 移动无人机"_qs;
        l.gridW = 3; l.gridH = 1;
        l.maxTimeSec = 300;
        l.droneMaxEnergy = 20.0f;
        l.bugProbability = 0.0f;
        l.allowedFunctions = {"move","harvest","get_pos","get_map_size"};
        l.allowedSyntax = {"expr","call"};
        l.allowedCrops = {"wheat"};
        l.tutorialCode = u"# 三格小麦，需要移动收割\n# move(\"right\") 向右移动\n\nharvest()\nmove(\"right\")\nharvest()\nmove(\"right\")\nharvest()\n"_qs;
        Cell c0; c0.x=0; c0.y=0; c0.state=CellState::Mature; c0.crop=CropType::Wheat; c0.progress=1.0f; c0.water=0.5f;
        Cell c1; c1.x=1; c1.y=0; c1.state=CellState::Mature; c1.crop=CropType::Wheat; c1.progress=1.0f; c1.water=0.5f;
        Cell c2; c2.x=2; c2.y=0; c2.state=CellState::Mature; c2.crop=CropType::Wheat; c2.progress=1.0f; c2.water=0.5f;
        l.presetCells = {c0, c1, c2};
        l.goals = {
            {u"收获小麦 × 3"_qs, GoalType::HarvestCount, "wheat", 3, 0, false, StarTier::Star1},
            {u"60 秒内完成"_qs, GoalType::TimeLimit, "", 60, 0, false, StarTier::Star2},
            {u"15 秒内完成"_qs, GoalType::TimeLimit, "", 15, 0, false, StarTier::Star3}
        };
        levels_[2] = l;
    }

    // === 关卡 3：浇水的烦恼 (3×3, 胡萝卜需浇水) ===
    {
        LevelConfig l;
        l.levelId = 3;
        l.name = u"浇水的烦恼"_qs;
        l.description = u"胡萝卜需要浇水才能成长"_qs;
        l.gridW = 3; l.gridH = 3;
        l.droneStartX = 1; l.droneStartY = 1;
        l.maxTimeSec = 120;
        l.droneMaxEnergy = 20.0f;
        l.bugProbability = 0.0f;
        l.allowedFunctions = {"move","harvest","water","wait","get_pos","get_map_size"};
        l.allowedSyntax = {"expr","call","assign"};
        l.allowedCrops = {"carrot"};
        l.tutorialCode = u"# 先浇水，这一回合结束后胡萝卜会成熟\n# 下一回合再 harvest()\n\nwater()\nharvest()\n"_qs;
        Cell c; c.x=1; c.y=1; c.state=CellState::Planted;
        c.crop=CropType::Carrot; c.progress=0.97f; c.water=0.1f;
        l.presetCells = {c};
        l.goals = {
            {u"收获胡萝卜 × 1"_qs, GoalType::HarvestCount, "carrot", 1, 0, false, StarTier::Star1},
            {u"60 秒内完成"_qs, GoalType::TimeLimit, "", 60, 0, false, StarTier::Star2},
            {u"20 秒内完成"_qs, GoalType::TimeLimit, "", 20, 0, false, StarTier::Star3}
        };
        levels_[3] = l;
    }

    // === 关卡 4：感知与循环 (3×1, 用 for 循环) ===
    {
        LevelConfig l;
        l.levelId = 4;
        l.name = u"感知与循环"_qs;
        l.description = u"用 for 循环批量收割"_qs;
        l.gridW = 3; l.gridH = 1;
        l.maxTimeSec = 180;
        l.droneMaxEnergy = 20.0f;
        l.bugProbability = 0.0f;
        l.allowedFunctions = {"move","harvest","wait","get_pos","get_map_size"};
        l.allowedSyntax = {"expr","call","assign","for","in","range"};
        l.allowedCrops = {"wheat"};
        l.tutorialCode = u"# 用 for 循环处理前两格，再收最后一格\n\nfor i in range(2):\n    harvest()\n    move(\"right\")\n\nharvest()\n"_qs;
        Cell c0; c0.x=0; c0.y=0; c0.state=CellState::Mature; c0.crop=CropType::Wheat; c0.progress=1.0f; c0.water=0.5f;
        Cell c1; c1.x=1; c1.y=0; c1.state=CellState::Mature; c1.crop=CropType::Wheat; c1.progress=1.0f; c1.water=0.5f;
        Cell c2; c2.x=2; c2.y=0; c2.state=CellState::Mature; c2.crop=CropType::Wheat; c2.progress=1.0f; c2.water=0.5f;
        l.presetCells = {c0, c1, c2};
        l.goals = {
            {u"收获小麦 × 3"_qs, GoalType::HarvestCount, "wheat", 3, 0, false, StarTier::Star1},
            {u"90 秒内完成"_qs, GoalType::TimeLimit, "", 90, 0, false, StarTier::Star2},
            {u"45 秒内完成"_qs, GoalType::TimeLimit, "", 45, 0, false, StarTier::Star3}
        };
        levels_[4] = l;
    }

    // === 关卡 5：施肥竞速 (2×2, 施肥加速) ===
    {
        LevelConfig l;
        l.levelId = 5;
        l.name = u"施肥竞速"_qs;
        l.description = u"施肥让作物成长加速 1.5-1.8 倍"_qs;
        l.gridW = 2; l.gridH = 2;
        l.maxTimeSec = 120;
        l.droneMaxEnergy = 30.0f;
        l.bugProbability = 0.0f;
        l.allowedFunctions = {"move","till","plant","harvest","water","fertilize","wait","get_pos","get_map_size"};
        l.allowedSyntax = {"expr","call","assign","for","in","range","if"};
        l.allowedCrops = {"wheat","carrot"};
        l.tutorialCode = u"# 目标是收获 4 株作物\n# 2x2 地图适合蛇形路线，成长期间可用 wait() 消耗 tick\n"_qs;
        l.presetCells = {};
        l.goals = {
            {u"收获作物 × 4"_qs, GoalType::HarvestCount, "", 4, 0, false, StarTier::Star1},
            {u"60 秒内完成"_qs, GoalType::TimeLimit, "", 60, 0, false, StarTier::Star2},
            {u"30 秒内完成"_qs, GoalType::TimeLimit, "", 30, 0, false, StarTier::Star3}
        };
        levels_[5] = l;
    }

    // === 关卡 6：玉米农场 (3×3, 玉米生长慢) ===
    {
        LevelConfig l;
        l.levelId = 6;
        l.name = u"玉米农场"_qs;
        l.description = u"玉米生长周期长，合理规划时间"_qs;
        l.gridW = 3; l.gridH = 3;
        l.maxTimeSec = 240;
        l.droneMaxEnergy = 30.0f;
        l.bugProbability = 0.005f;
        l.allowedFunctions = {"move","till","plant","harvest","water","fertilize","spray","wait","debug","get_pos","get_map_size","get_current"};
        l.allowedSyntax = {"expr","call","assign","for","in","range","if","else","while","def"};
        l.allowedCrops = {"wheat","carrot","corn"};
        l.tutorialCode = u"# 玉米需要 50 tick 成长\n# debug() / get_current() 可查看当前格状态\n# wait() 消耗一个 tick，spray() 可处理虫害\n"_qs;
        l.presetCells = {};
        l.goals = {
            {u"收获玉米 × 2"_qs, GoalType::HarvestCount, "corn", 2, 0, false, StarTier::Star1},
            {u"120 秒内完成"_qs, GoalType::TimeLimit, "", 120, 0, false, StarTier::Star2},
            {u"60 秒内完成"_qs, GoalType::TimeLimit, "", 60, 0, false, StarTier::Star3}
        };
        levels_[6] = l;
    }

    // === 关卡 7：混合种植 (4×4, 多种作物) ===
    {
        LevelConfig l;
        l.levelId = 7;
        l.name = u"混合种植"_qs;
        l.description = u"种植多种作物，合理分配资源"_qs;
        l.gridW = 4; l.gridH = 4;
        l.maxTimeSec = 300;
        l.droneMaxEnergy = 40.0f;
        l.bugProbability = 0.008f;
        l.allowedFunctions = {"move","till","plant","harvest","water","fertilize","spray","wait","debug","get_pos","get_map_size","get_current","get_energy"};
        l.allowedSyntax = {"expr","call","assign","for","in","range","if","else","while","def","not","and","or"};
        l.allowedCrops = {"wheat","carrot","tomato","corn"};
        l.tutorialCode = u"# 混合种植：小麦快、胡萝卜需水、番茄均衡、玉米慢\n# wait() 推进时间，spray() 清理虫害\n\ndef plant_row(crop):\n    for i in range(4):\n        till()\n        plant(crop)\n        water()\n        if i < 3:\n            move(\"right\")\n"_qs;
        l.presetCells = {};
        l.goals = {
            {u"收获小麦 × 4"_qs, GoalType::HarvestCount, "wheat", 4, 0, false, StarTier::Star1},
            {u"收获胡萝卜 × 2"_qs, GoalType::HarvestCount, "carrot", 2, 0, false, StarTier::Star2},
            {u"收获番茄 × 2"_qs, GoalType::HarvestCount, "tomato", 2, 0, false, StarTier::Star3}
        };
        levels_[7] = l;
    }

    // === 关卡 8：向日葵的赌注 (5×5, 高风险高回报) ===
    {
        LevelConfig l;
        l.levelId = 8;
        l.name = u"向日葵的赌注"_qs;
        l.description = u"向日葵生长极慢但价值最高，Bug概率高"_qs;
        l.gridW = 5; l.gridH = 5;
        l.maxTimeSec = 600;
        l.droneMaxEnergy = 50.0f;
        l.bugProbability = 0.015f;
        l.allowedFunctions = {"move","till","plant","harvest","water","fertilize","spray","wait","debug","get_pos","get_map_size","get_current","get_energy","get_tick","get_goals"};
        l.allowedSyntax = {"expr","call","assign","for","in","range","if","else","while","def","not","and","or","break","continue","return"};
        l.allowedCrops = {"wheat","carrot","tomato","corn","sunflower"};
        l.tutorialCode = u"# 向日葵：90 tick 成长，虫害概率高\n# 使用 wait() 推进 tick，get_tick()/get_goals() 查询状态\n# spray() 可以清除虫害并获得短暂免疫\n"_qs;
        l.presetCells = {};
        l.goals = {
            {u"收获向日葵 × 1"_qs, GoalType::HarvestCount, "sunflower", 1, 0, false, StarTier::Star1},
            {u"收获向日葵 × 3"_qs, GoalType::HarvestCount, "sunflower", 3, 0, false, StarTier::Star2},
            {u"300 秒内完成"_qs, GoalType::TimeLimit, "", 300, 0, false, StarTier::Star3}
        };
        levels_[8] = l;
    }
}

LevelConfig LevelManager::getLevelConfig(int id) const {
    return levels_.value(id);
}

QVariantMap LevelManager::getLevel(int id) const {
    if (!levels_.contains(id)) return {};
    const auto &l = levels_[id];
    QVariantList goals;
    for (const auto &goal : l.goals) {
        goals.append(QVariantMap{
            {"description", goal.description},
            {"target", goal.targetValue},
            {"starTier", static_cast<int>(goal.starTier)}
        });
    }
    QVariantList allowedFunctions;
    QStringList allowedFunctionNames = l.allowedFunctions.values();
    allowedFunctionNames.sort();
    for (const auto &func : allowedFunctionNames) {
        allowedFunctions.append(func);
    }
    return {
        {"id", l.levelId}, {"name", l.name},
        {"description", l.description},
        {"gridW", l.gridW}, {"gridH", l.gridH},
        {"maxTimeSec", l.maxTimeSec},
        {"goals", goals},
        {"allowedFunctions", allowedFunctions}
    };
}

bool LevelManager::isUnlocked(int id) const {
    return progress_.value(id).unlocked;
}

int LevelManager::getStars(int id) const {
    return progress_.value(id).stars;
}

int LevelManager::getBestTime(int id) const {
    return progress_.value(id).bestTime;
}

int LevelManager::getClearCount(int id) const {
    return progress_.value(id).clearCount;
}

int LevelManager::totalStars() const {
    int total = 0;
    for (const auto &p : progress_) total += p.stars;
    return total;
}

int LevelManager::completedCount() const {
    int count = 0;
    for (const auto &p : progress_)
        if (p.stars > 0) count++;
    return count;
}

void LevelManager::recordClear(int id, int timeUsed, int starsEarned) {
    auto &p = progress_[id];
    p.unlocked = true;
    if (starsEarned > p.stars) p.stars = starsEarned;
    if (p.bestTime < 0 || timeUsed < p.bestTime) p.bestTime = timeUsed;
    p.clearCount++;

    const int nextId = id + 1;
    if (nextId <= levels_.size() && !progress_[nextId].unlocked) {
        progress_[nextId].unlocked = true;
        emit levelUnlocked(nextId);
    }
    emit progressChanged();
}

void LevelManager::loadProgress(const QJsonObject &savedProgress) {
    progress_.clear();
    progress_[1].unlocked = true;

    const auto keys = savedProgress.keys();
    for (const QString &key : keys) {
        bool ok = false;
        const int id = key.toInt(&ok);
        if (!ok || !levels_.contains(id)) {
            continue;
        }

        const QJsonObject data = savedProgress.value(key).toObject();
        auto &p = progress_[id];
        p.unlocked = true;
        p.stars = data.value(QStringLiteral("stars")).toInt();
        p.bestTime = data.value(QStringLiteral("bestTime")).toInt(-1);
        p.clearCount = data.value(QStringLiteral("clearCount")).toInt();

        if (p.stars > 0) {
            const int nextId = id + 1;
            if (levels_.contains(nextId)) {
                progress_[nextId].unlocked = true;
            }
        }
    }

    emit progressChanged();
}
