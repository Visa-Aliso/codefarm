#include "levelmanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

using namespace Qt::StringLiterals;

// ============================================================================
// 辅助函数：创建岩石和虫害区预设格子
// ============================================================================
namespace {

Cell makeRock(int x, int y) {
    Cell c;
    c.x = x; c.y = y;
    c.state = CellState::Tilled; // init() 中会转为 Rock
    c.tilledTicks = -1;          // 岩石标记
    return c;
}

Cell makePestZone(int x, int y) {
    Cell c;
    c.x = x; c.y = y;
    c.state = CellState::Empty;
    c.bugImmuneTicks = -2;       // 虫害区标记
    return c;
}

} // anonymous namespace

LevelManager::LevelManager(QObject *parent) : QObject(parent) {
    loadLevels();
    progress_[1].unlocked = true;
}

void LevelManager::loadLevels() {
    // =====================================================================
    // 阶段1：入门耕种（关1-2）— 最基础的耕种流程
    // =====================================================================

    // === 关卡 1：初次耕种 (1×1) ===
    // 教学：完整的 till→plant→water→wait→harvest 流程
    {
        LevelConfig l;
        l.levelId = 1;
        l.name = u"初次耕种"_s;
        l.description = u"学习完整的耕种流程：犁地→种植→浇水→等待→收割。目标：收获小麦×1"_s;
        l.gridW = 1; l.gridH = 1;
        l.maxTimeSec = 300;

        l.bugProbability = 0.0f;
        l.allowedFunctions = {"till","plant","harvest","water","wait"};
        l.allowedSyntax = {"expr","call","for","in","range"};
        l.allowedCrops = {"wheat"};
        l.tutorialCode = u"# 完整的耕种流程\n\n# 1. 犁地\ntill()\n# 2. 种植小麦\nplant(\"wheat\")\n# 3. 浇水\nwater()\n# 4. 等待成熟（小麦需要6 tick）\nfor i in range(6):\n    wait()\n# 5. 收割\nharvest()\n"_s;
        l.presetCells = {};
        l.goals = {
            {u"收获小麦 × 1"_s, GoalType::HarvestCount, "wheat", 1, 0, false, StarTier::Star1},
            {u"40 秒内完成"_s, GoalType::TimeLimit, "", 40, 0, false, StarTier::Star2},
            {u"20 秒内完成"_s, GoalType::TimeLimit, "", 20, 0, false, StarTier::Star3}
        };
        levels_[1] = l;
    }

    // === 关卡 2：田字初耕 (2×2) ===
    // 教学：二维移动 + 多格耕种
    {
        LevelConfig l;
        l.levelId = 2;
        l.name = u"田字初耕"_s;
        l.description = u"在2×2网格上学习二维移动和耕种。目标：收获小麦×4"_s;
        l.gridW = 2; l.gridH = 2;
        l.maxTimeSec = 300;

        l.bugProbability = 0.0f;
        l.allowedFunctions = {"move","till","plant","harvest","water","wait","get_pos","get_map_size"};
        l.allowedSyntax = {"expr","call","if","for","in","range"};
        l.allowedCrops = {"wheat"};
        l.tutorialCode = u"# 2×2 网格耕种\nfor row in range(2):\n    for col in range(2):\n        till()\n        plant(\"wheat\")\n        water()\n        if col < 1:\n            move(\"right\")\n    if row < 1:\n        move(\"down\")\n        move(\"left\")\n\n# 回到起点（先左再上）\nmove(\"left\")\nmove(\"up\")\n\n# 等待成熟\nfor i in range(6):\n    wait()\n\n# 收割\nfor row in range(2):\n    for col in range(2):\n        harvest()\n        if col < 1:\n            move(\"right\")\n    if row < 1:\n        move(\"down\")\n        move(\"left\")\n"_s;
        l.presetCells = {};
        l.goals = {
            {u"收获小麦 × 4"_s, GoalType::HarvestCount, "wheat", 4, 0, false, StarTier::Star1},
            {u"90 秒内完成"_s, GoalType::TimeLimit, "", 90, 0, false, StarTier::Star2},
            {u"50 秒内完成"_s, GoalType::TimeLimit, "", 50, 0, false, StarTier::Star3}
        };
        levels_[2] = l;
    }

    // =====================================================================
    // 阶段2：基础循环（关3-5）— for循环 + 条件 + 函数
    // =====================================================================

    // === 关卡 3：for 循环 (3×3) ===
    // 教学：for + range 简化重复代码，遇到障碍物绕行
    {
        LevelConfig l;
        l.levelId = 3;
        l.name = u"循环耕田"_s;
        l.description = u"用 for 循环在3×3网格上批量耕种，注意绕开岩石。目标：收获小麦×8"_s;
        l.gridW = 3; l.gridH = 3;
        l.maxTimeSec = 300;

        l.bugProbability = 0.0f;
        l.allowedFunctions = {"move","till","plant","harvest","water","wait","get_pos","get_map_size","get_current"};
        l.allowedSyntax = {"expr","call","assign","if","for","in","range","while","def"};
        l.allowedCrops = {"wheat"};
        l.tutorialCode = u"# 用循环耕种，绕开岩石\n# 岩石在 (1,1)，即正中间\n# 用 get_current() 检查格子状态\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n    while get_pos()[1] > 0:\n        move(\"up\")\n\nfor row in range(3):\n    go_home()\n    for i in range(row):\n        move(\"down\")\n    for col in range(3):\n        cell = get_current()\n        if cell[\"state\"] == \"empty\":\n            till()\n            plant(\"wheat\")\n            water()\n        if col < 2:\n            move(\"right\")\n\ngo_home()\n\n# 等待\nfor i in range(6):\n    wait()\n\n# 收割\nfor row in range(3):\n    go_home()\n    for i in range(row):\n        move(\"down\")\n    for col in range(3):\n        cell = get_current()\n        if cell[\"state\"] == \"mature\":\n            harvest()\n        if col < 2:\n            move(\"right\")\n"_s;
        l.presetCells = {makeRock(2, 2)};
        l.goals = {
            {u"收获小麦 × 8"_s, GoalType::HarvestCount, "wheat", 8, 0, false, StarTier::Star1},
            {u"120 秒内完成"_s, GoalType::TimeLimit, "", 120, 0, false, StarTier::Star2},
            {u"70 秒内完成"_s, GoalType::TimeLimit, "", 70, 0, false, StarTier::Star3}
        };
        levels_[3] = l;
    }

    // === 关卡 4：认识胡萝卜 (3×3) ===
    // 教学：胡萝卜需要更多水，且过涝会受伤
    {
        LevelConfig l;
        l.levelId = 4;
        l.name = u"认识胡萝卜"_s;
        l.description = u"胡萝卜需要更多水分，但浇水过多会过涝受伤！目标：收获小麦×2、胡萝卜×6"_s;
        l.gridW = 3; l.gridH = 3;
        l.maxTimeSec = 300;

        l.bugProbability = 0.0f;
        l.allowedFunctions = {"move","till","plant","harvest","water","wait","get_pos","get_map_size","get_current"};
        l.allowedSyntax = {"expr","call","assign","if","for","in","range","def","while","else"};
        l.allowedCrops = {"wheat","carrot"};
        l.tutorialCode = u"# 胡萝卜需要精准浇水\n# 浇两次水刚好合适，三次就会过涝！\n# 种满所有格子：2小麦 + 6胡萝卜\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\n# 种满所有格子（跳过岩石）\nfor row in range(3):\n    for col in range(3):\n        cell = get_current()\n        if cell[\"state\"] == \"empty\":\n            till()\n            if row == 0:\n                plant(\"wheat\")\n                water()\n            else:\n                plant(\"carrot\")\n                water()\n                water()  # 胡萝卜需要两次水\n        if col < 2:\n            move(\"right\")\n    go_home()\n    if row < 2:\n        move(\"down\")\n\ngo_home()\nwhile get_pos()[1] > 0:\n    move(\"up\")\n\n# 等待（胡萝卜需要14 tick）\nfor i in range(14):\n    wait()\n\n# 收割所有作物\nfor row in range(3):\n    for col in range(3):\n        cell = get_current()\n        if cell[\"state\"] == \"mature\":\n            harvest()\n        if col < 2:\n            move(\"right\")\n    go_home()\n    if row < 2:\n        move(\"down\")\n"_s;
        l.presetCells = {makeRock(2, 0)};
        l.goals = {
            {u"收获小麦 × 2"_s, GoalType::HarvestCount, "wheat", 2, 0, false, StarTier::Star1},
            {u"收获胡萝卜 × 6"_s, GoalType::HarvestCount, "carrot", 6, 0, false, StarTier::Star1},
            {u"150 秒内完成"_s, GoalType::TimeLimit, "", 150, 0, false, StarTier::Star2},
            {u"100 秒内完成"_s, GoalType::TimeLimit, "", 100, 0, false, StarTier::Star3}
        };
        levels_[4] = l;
    }

    // === 关卡 5：函数封装 (4×4) ===
    // 教学：def 封装重复逻辑
    {
        LevelConfig l;
        l.levelId = 5;
        l.name = u"函数封装"_s;
        l.description = u"用 def 定义函数封装耕种流程。目标：收获小麦×8、胡萝卜×8"_s;
        l.gridW = 4; l.gridH = 4;
        l.maxTimeSec = 300;

        l.bugProbability = 0.0f;
        l.allowedFunctions = {"move","till","plant","harvest","water","wait","get_pos","get_map_size","get_current"};
        l.allowedSyntax = {"expr","call","assign","if","for","in","range","def","while","else"};
        l.allowedCrops = {"wheat","carrot"};
        l.tutorialCode = u"# 用函数封装耕种\n# 种满所有16格：8小麦 + 8胡萝卜\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\ndef go_top():\n    while get_pos()[1] > 0:\n        move(\"up\")\n\ndef plant_row(crop):\n    for i in range(4):\n        cell = get_current()\n        if cell[\"state\"] == \"empty\":\n            till()\n            plant(crop)\n            water()\n            if crop == \"carrot\":\n                water()\n        if i < 3:\n            move(\"right\")\n\ndef harvest_row():\n    for i in range(4):\n        cell = get_current()\n        if cell[\"state\"] == \"mature\":\n            harvest()\n        if i < 3:\n            move(\"right\")\n\n# 种两行小麦 + 两行胡萝卜\nfor row in range(4):\n    if row < 2:\n        plant_row(\"wheat\")\n    else:\n        plant_row(\"carrot\")\n    go_home()\n    if row < 3:\n        move(\"down\")\n\ngo_home()\ngo_top()\n\n# 等待（胡萝卜需要14 tick）\nfor i in range(14):\n    wait()\n\n# 收割所有\nfor row in range(4):\n    harvest_row()\n    go_home()\n    if row < 3:\n        move(\"down\")\n"_s;
        l.presetCells = {};
        l.goals = {
            {u"收获小麦 × 8"_s, GoalType::HarvestCount, "wheat", 8, 0, false, StarTier::Star1},
            {u"收获胡萝卜 × 8"_s, GoalType::HarvestCount, "carrot", 8, 0, false, StarTier::Star1},
            {u"180 秒内完成"_s, GoalType::TimeLimit, "", 180, 0, false, StarTier::Star2},
            {u"120 秒内完成"_s, GoalType::TimeLimit, "", 120, 0, false, StarTier::Star3}
        };
        levels_[5] = l;
    }

    // =====================================================================
    // 阶段3：进阶管理（关6-8）— 施肥 + 番茄 + while
    // =====================================================================

    // === 关卡 6：施肥加速 (4×4) ===
    // 教学：fertilize 加速生长
    {
        LevelConfig l;
        l.levelId = 6;
        l.name = u"施肥加速"_s;
        l.description = u"施肥可加速作物生长 1.5-1.8 倍。目标：收获小麦×14"_s;
        l.gridW = 4; l.gridH = 4;
        l.maxTimeSec = 180;

        l.bugProbability = 0.0f;
        l.allowedFunctions = {"move","till","plant","harvest","water","fertilize","wait","get_pos","get_map_size","get_current"};
        l.allowedSyntax = {"expr","call","assign","if","for","in","range","def","while"};
        l.allowedCrops = {"wheat","carrot"};
        l.tutorialCode = u"# 施肥加速收获！\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\n# 种植并施肥\nfor row in range(4):\n    for col in range(4):\n        cell = get_current()\n        if cell[\"state\"] == \"empty\":\n            till()\n            plant(\"wheat\")\n            water()\n            fertilize()\n        if col < 3:\n            move(\"right\")\n    go_home()\n    if row < 3:\n        move(\"down\")\n\n# 回到起点\ngo_home()\nwhile get_pos()[1] > 0:\n    move(\"up\")\n\n# 施肥后小麦约4 tick成熟\nfor i in range(5):\n    wait()\n\n# 收割\nfor row in range(4):\n    for col in range(4):\n        cell = get_current()\n        if cell[\"state\"] == \"mature\":\n            harvest()\n        if col < 3:\n            move(\"right\")\n    go_home()\n    if row < 3:\n        move(\"down\")\n"_s;
        l.presetCells = {makeRock(3, 0), makeRock(3, 2)};
        l.goals = {
            {u"收获小麦 × 14"_s, GoalType::HarvestCount, "wheat", 14, 0, false, StarTier::Star1},
            {u"120 秒内完成"_s, GoalType::TimeLimit, "", 120, 0, false, StarTier::Star2},
            {u"80 秒内完成"_s, GoalType::TimeLimit, "", 80, 0, false, StarTier::Star3}
        };
        levels_[6] = l;
    }

    // === 关卡 7：番茄挑战 (4×4) ===
    // 教学：番茄需要二次浇水
    {
        LevelConfig l;
        l.levelId = 7;
        l.name = u"番茄挑战"_s;
        l.description = u"番茄初始水分较少，需要更早补浇。目标：收获小麦×3、胡萝卜×3、番茄×7"_s;
        l.gridW = 4; l.gridH = 4;
        l.maxTimeSec = 300;

        l.bugProbability = 0.0f;
        l.allowedFunctions = {"move","till","plant","harvest","water","fertilize","wait","get_pos","get_map_size","get_current"};
        l.allowedSyntax = {"expr","call","assign","if","for","in","range","def","while","else"};
        l.allowedCrops = {"wheat","carrot","tomato"};
        l.tutorialCode = u"# 种满所有格子：小麦+胡萝卜+番茄\n# 岩石在 (3,0), (3,1), (2,3)\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\ndef go_top():\n    while get_pos()[1] > 0:\n        move(\"up\")\n\ndef plant_row(crop):\n    for i in range(4):\n        cell = get_current()\n        if cell[\"state\"] == \"empty\":\n            till()\n            plant(crop)\n            water()\n            if crop == \"carrot\":\n                water()\n            fertilize()\n        if i < 3:\n            move(\"right\")\n\ndef harvest_row():\n    for i in range(4):\n        cell = get_current()\n        if cell[\"state\"] == \"mature\":\n            harvest()\n        if i < 3:\n            move(\"right\")\n\n# 种四行（跳过岩石）\nfor row in range(4):\n    if row == 0:\n        plant_row(\"wheat\")\n    elif row == 1:\n        plant_row(\"carrot\")\n    else:\n        plant_row(\"tomato\")\n    go_home()\n    if row < 3:\n        move(\"down\")\n\ngo_home()\ngo_top()\n\n# 等待成熟\nfor i in range(10):\n    wait()\n\n# 收割所有\nfor row in range(4):\n    harvest_row()\n    go_home()\n    if row < 3:\n        move(\"down\")\n"_s;
        l.presetCells = {makeRock(3, 0), makeRock(3, 1), makeRock(3, 3)};
        l.goals = {
            {u"收获小麦 × 3"_s, GoalType::HarvestCount, "wheat", 3, 0, false, StarTier::Star1},
            {u"收获胡萝卜 × 3"_s, GoalType::HarvestCount, "carrot", 3, 0, false, StarTier::Star1},
            {u"收获番茄 × 7"_s, GoalType::HarvestCount, "tomato", 7, 0, false, StarTier::Star1},
            {u"180 秒内完成"_s, GoalType::TimeLimit, "", 180, 0, false, StarTier::Star2},
            {u"120 秒内完成"_s, GoalType::TimeLimit, "", 120, 0, false, StarTier::Star3}
        };
        levels_[7] = l;
    }

    // === 关卡 8：循环优化 (4×4) ===
    // 教学：while 循环持续巡逻
    {
        LevelConfig l;
        l.levelId = 8;
        l.name = u"循环优化"_s;
        l.description = u"用 while 循环优化耕种和巡逻逻辑。目标：收获小麦×6、番茄×6"_s;
        l.gridW = 4; l.gridH = 4;
        l.maxTimeSec = 300;

        l.bugProbability = 0.0f;
        l.allowedFunctions = {"move","till","plant","harvest","water","fertilize","wait","get_pos","get_map_size","get_current"};
        l.allowedSyntax = {"expr","call","assign","if","for","in","range","def","while","else"};
        l.allowedCrops = {"wheat","carrot","tomato"};
        l.tutorialCode = u"# 用 while 优化路径\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\ndef go_top():\n    while get_pos()[1] > 0:\n        move(\"up\")\n\ndef plant_all():\n    for row in range(4):\n        for col in range(4):\n            cell = get_current()\n            if cell[\"state\"] == \"empty\":\n                till()\n                if row < 2:\n                    plant(\"wheat\")\n                else:\n                    plant(\"tomato\")\n                water()\n                fertilize()\n            if col < 3:\n                move(\"right\")\n        go_home()\n        if row < 3:\n            move(\"down\")\n\ndef harvest_all():\n    for row in range(4):\n        for col in range(4):\n            cell = get_current()\n            if cell[\"state\"] == \"mature\":\n                harvest()\n            if col < 3:\n                move(\"right\")\n        go_home()\n        if row < 3:\n            move(\"down\")\n\nplant_all()\ngo_home()\ngo_top()\n\nfor i in range(8):\n    wait()\n\nharvest_all()\n"_s;
        l.presetCells = {makeRock(3, 0), makeRock(3, 1), makeRock(3, 2), makeRock(3, 3)};
        l.goals = {
            {u"收获小麦 × 6"_s, GoalType::HarvestCount, "wheat", 6, 0, false, StarTier::Star1},
            {u"收获番茄 × 6"_s, GoalType::HarvestCount, "tomato", 6, 0, false, StarTier::Star1},
            {u"150 秒内完成"_s, GoalType::TimeLimit, "", 150, 0, false, StarTier::Star2},
            {u"100 秒内完成"_s, GoalType::TimeLimit, "", 100, 0, false, StarTier::Star3}
        };
        levels_[8] = l;
    }

    // =====================================================================
    // 阶段4：虫害管理（关9-11）— spray + 玉米 + 巡逻
    // =====================================================================

    // === 关卡 9：除虫入门 (4×4) ===
    // 教学：spray() 杀虫，虫害区概念
    {
        LevelConfig l;
        l.levelId = 9;
        l.name = u"除虫入门"_s;
        l.description = u"虫害会出现！学习用 spray() 保护作物。目标：收获番茄×14"_s;
        l.gridW = 4; l.gridH = 4;
        l.maxTimeSec = 300;

        l.bugProbability = 0.003f;
        l.allowedFunctions = {"move","till","plant","harvest","water","fertilize","spray","wait","debug","get_pos","get_map_size","get_current"};
        l.allowedSyntax = {"expr","call","assign","if","for","in","range","def","while","else"};
        l.allowedCrops = {"wheat","carrot","tomato"};
        l.tutorialCode = u"# 种番茄，巡逻防虫并浇水\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\ndef go_top():\n    while get_pos()[1] > 0:\n        move(\"up\")\n\n# 种两行番茄\nfor row in range(2):\n    for col in range(4):\n        cell = get_current()\n        if cell[\"state\"] == \"empty\":\n            till()\n            plant(\"tomato\")\n            water()\n            fertilize()\n        if col < 3:\n            move(\"right\")\n    go_home()\n    if row < 1:\n        move(\"down\")\n\n# 回到起点\ngo_home()\ngo_top()\n\n# 巡逻防虫并浇水\nfor t in range(15):\n    wait()\n    if t % 3 == 0:\n        for row in range(2):\n            for col in range(4):\n                cell = get_current()\n                if cell[\"hasBug\"]:\n                    spray()\n                elif cell[\"water\"] < 0.2:\n                    water()\n                if col < 3:\n                    move(\"right\")\n            go_home()\n            if row < 1:\n                move(\"down\")\n        go_home()\n        go_top()\n\n# 等待成熟\nfor i in range(5):\n    wait()\n\n# 收割\ngo_home()\ngo_top()\nfor row in range(2):\n    for col in range(4):\n        cell = get_current()\n        if cell[\"state\"] == \"mature\":\n            harvest()\n        if col < 3:\n            move(\"right\")\n    go_home()\n    if row < 1:\n        move(\"down\")\n"_s;
        l.presetCells = {makeRock(3, 2), makeRock(3, 3), makePestZone(2, 1)};
        l.goals = {
            {u"收获番茄 × 14"_s, GoalType::HarvestCount, "tomato", 14, 0, false, StarTier::Star1},
            {u"240 秒内完成"_s, GoalType::TimeLimit, "", 240, 0, false, StarTier::Star2},
            {u"150 秒内完成"_s, GoalType::TimeLimit, "", 150, 0, false, StarTier::Star3}
        };
        levels_[9] = l;
    }

    // === 关卡 10：玉米种植 (5×5) ===
    // 教学：玉米生长慢，旱灾惩罚，持续供水
    {
        LevelConfig l;
        l.levelId = 10;
        l.name = u"玉米种植"_s;
        l.description = u"玉米生长慢且缺水会损失进度，需要持续供水。目标：收获玉米×23"_s;
        l.gridW = 5; l.gridH = 5;
        l.maxTimeSec = 300;

        l.bugProbability = 0.005f;
        l.allowedFunctions = {"move","till","plant","harvest","water","fertilize","spray","wait","debug","get_pos","get_map_size","get_current"};
        l.allowedSyntax = {"expr","call","assign","if","for","in","range","def","while","else"};
        l.allowedCrops = {"wheat","carrot","tomato","corn"};
        l.tutorialCode = u"# 种玉米，施肥加速，巡逻防虫\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\ndef go_top():\n    while get_pos()[1] > 0:\n        move(\"up\")\n\n# 种两行玉米并施肥\nfor row in range(2):\n    for col in range(4):\n        cell = get_current()\n        if cell[\"state\"] == \"empty\":\n            till()\n            plant(\"corn\")\n            water()\n            water()  # 多浇水防干旱\n            fertilize()\n        if col < 3:\n            move(\"right\")\n    go_home()\n    if row < 1:\n        move(\"down\")\n\n# 回到起点\ngo_home()\ngo_top()\n\n# 巡逻防虫并补水\nfor t in range(20):\n    wait()\n    if t % 3 == 0:\n        for row in range(2):\n            for col in range(4):\n                cell = get_current()\n                if cell[\"hasBug\"]:\n                    spray()\n                elif cell[\"water\"] < 0.25:\n                    water()\n                if col < 3:\n                    move(\"right\")\n            go_home()\n            if row < 1:\n                move(\"down\")\n        go_home()\n        go_top()\n\n# 收割\ngo_home()\ngo_top()\nfor row in range(2):\n    for col in range(4):\n        cell = get_current()\n        if cell[\"state\"] == \"mature\":\n            harvest()\n        if col < 3:\n            move(\"right\")\n    go_home()\n    if row < 1:\n        move(\"down\")\n"_s;
        l.presetCells = {makeRock(4, 1), makeRock(4, 4), makePestZone(1, 0), makePestZone(3, 1)};
        l.goals = {
            {u"收获玉米 × 23"_s, GoalType::HarvestCount, "corn", 23, 0, false, StarTier::Star1},
            {u"240 秒内完成"_s, GoalType::TimeLimit, "", 240, 0, false, StarTier::Star2},
            {u"150 秒内完成"_s, GoalType::TimeLimit, "", 150, 0, false, StarTier::Star3}
        };
        levels_[10] = l;
    }

    // === 关卡 11：巡逻防线 (5×5) ===
    // 教学：持续巡逻模式，多作物管理
    {
        LevelConfig l;
        l.levelId = 11;
        l.name = u"巡逻防线"_s;
        l.description = u"用循环持续巡逻，保护所有作物不受虫害。目标：收获小麦×8、番茄×7、玉米×7"_s;
        l.gridW = 5; l.gridH = 5;
        l.maxTimeSec = 300;

        l.bugProbability = 0.007f;
        l.allowedFunctions = {"move","till","plant","harvest","water","fertilize","spray","wait","debug","get_pos","get_map_size","get_current"};
        l.allowedSyntax = {"expr","call","assign","if","for","in","range","def","while","else"};
        l.allowedCrops = {"wheat","carrot","tomato","corn"};
        l.tutorialCode = u"# 混合种植，持续巡逻\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\ndef go_top():\n    while get_pos()[1] > 0:\n        move(\"up\")\n\ndef plant_row(crop):\n    for i in range(5):\n        cell = get_current()\n        if cell[\"state\"] == \"empty\":\n            till()\n            plant(crop)\n            water()\n            if crop == \"carrot\":\n                water()\n            fertilize()\n        if i < 4:\n            move(\"right\")\n\nplant_row(\"wheat\")\nmove(\"down\")\ngo_home()\nplant_row(\"tomato\")\nmove(\"down\")\ngo_home()\nplant_row(\"corn\")\n\n# 回到起点\ngo_home()\ngo_top()\n\n# 持续巡逻（每3 tick巡逻一次）\ndef patrol():\n    for row in range(3):\n        for col in range(5):\n            cell = get_current()\n            if cell[\"hasBug\"]:\n                spray()\n            elif cell[\"water\"] < 0.15:\n                water()\n            if col < 4:\n                move(\"right\")\n        go_home()\n        if row < 2:\n            move(\"down\")\n    go_home()\n    go_top()\n\nfor t in range(20):\n    wait()\n    if t % 3 == 0:\n        patrol()\n\n# 收割\ngo_home()\ngo_top()\nfor row in range(3):\n    for col in range(5):\n        cell = get_current()\n        if cell[\"state\"] == \"mature\":\n            harvest()\n        if col < 4:\n            move(\"right\")\n    go_home()\n    if row < 2:\n        move(\"down\")\n"_s;
        l.presetCells = {
            makeRock(4, 0), makeRock(4, 2), makeRock(0, 3),
            makePestZone(2, 1), makePestZone(4, 3)
        };
        l.goals = {
            {u"收获小麦 × 8"_s, GoalType::HarvestCount, "wheat", 8, 0, false, StarTier::Star1},
            {u"收获番茄 × 7"_s, GoalType::HarvestCount, "tomato", 7, 0, false, StarTier::Star1},
            {u"收获玉米 × 7"_s, GoalType::HarvestCount, "corn", 7, 0, false, StarTier::Star1},
            {u"240 秒内完成"_s, GoalType::TimeLimit, "", 240, 0, false, StarTier::Star2},
            {u"150 秒内完成"_s, GoalType::TimeLimit, "", 150, 0, false, StarTier::Star3}
        };
        levels_[11] = l;
    }

    // =====================================================================
    // 阶段5：高级逻辑（关12-14）— 布尔 + break/continue + return
    // =====================================================================

    // === 关卡 12：布尔逻辑 (5×5) ===
    // 教学：and/or/not, else
    {
        LevelConfig l;
        l.levelId = 12;
        l.name = u"布尔逻辑"_s;
        l.description = u"用 and、or、not 组合复杂条件判断。目标：收获小麦×6、胡萝卜×5、番茄×6、玉米×5"_s;
        l.gridW = 5; l.gridH = 5;
        l.maxTimeSec = 300;

        l.bugProbability = 0.008f;
        l.allowedFunctions = {"move","till","plant","harvest","water","fertilize","spray","wait","debug","get_pos","get_map_size","get_current"};
        l.allowedSyntax = {"expr","call","assign","if","for","in","range","def","while","else","not","and","or"};
        l.allowedCrops = {"wheat","carrot","tomato","corn"};
        l.tutorialCode = u"# 用布尔逻辑组合条件\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\ndef go_top():\n    while get_pos()[1] > 0:\n        move(\"up\")\n\ndef plant_row(crop):\n    for i in range(5):\n        cell = get_current()\n        if cell[\"state\"] == \"empty\":\n            till()\n            plant(crop)\n            water()\n            if crop == \"carrot\":\n                water()\n            fertilize()\n        if i < 4:\n            move(\"right\")\n\nplant_row(\"wheat\")\nmove(\"down\")\ngo_home()\nplant_row(\"carrot\")\nmove(\"down\")\ngo_home()\nplant_row(\"tomato\")\nmove(\"down\")\ngo_home()\nplant_row(\"corn\")\n\n# 回到起点\ngo_home()\ngo_top()\n\n# 用复合条件巡逻并收割\ndef smart_patrol():\n    for row in range(4):\n        for col in range(5):\n            cell = get_current()\n            if cell[\"hasBug\"]:\n                spray()\n            elif cell[\"state\"] == \"mature\" and not cell[\"hasBug\"]:\n                harvest()\n            elif cell[\"water\"] < 0.15:\n                water()\n            if col < 4:\n                move(\"right\")\n        go_home()\n        if row < 3:\n            move(\"down\")\n    go_home()\n    go_top()\n\nfor t in range(20):\n    wait()\n    if t % 3 == 0:\n        smart_patrol()\n\n# 最终收割\ngo_home()\ngo_top()\nfor row in range(4):\n    for col in range(5):\n        cell = get_current()\n        if cell[\"state\"] == \"mature\":\n            harvest()\n        if col < 4:\n            move(\"right\")\n    go_home()\n    if row < 3:\n        move(\"down\")\n"_s;
        l.presetCells = {
            makeRock(4, 0), makeRock(4, 1), makeRock(4, 2),
            makePestZone(1, 2), makePestZone(3, 0)
        };
        l.goals = {
            {u"收获小麦 × 6"_s, GoalType::HarvestCount, "wheat", 6, 0, false, StarTier::Star1},
            {u"收获胡萝卜 × 5"_s, GoalType::HarvestCount, "carrot", 5, 0, false, StarTier::Star1},
            {u"收获番茄 × 6"_s, GoalType::HarvestCount, "tomato", 6, 0, false, StarTier::Star1},
            {u"收获玉米 × 5"_s, GoalType::HarvestCount, "corn", 5, 0, false, StarTier::Star1},
            {u"240 秒内完成"_s, GoalType::TimeLimit, "", 240, 0, false, StarTier::Star2},
            {u"150 秒内完成"_s, GoalType::TimeLimit, "", 150, 0, false, StarTier::Star3}
        };
        levels_[12] = l;
    }

    // === 关卡 13：高级控制 (5×5) ===
    // 教学：break/continue 优化巡逻
    {
        LevelConfig l;
        l.levelId = 13;
        l.name = u"高级控制"_s;
        l.description = u"用 break 和 continue 优化巡逻逻辑。目标：收获玉米×21"_s;
        l.gridW = 5; l.gridH = 5;
        l.maxTimeSec = 240;

        l.bugProbability = 0.010f;
        l.allowedFunctions = {"move","till","plant","harvest","water","fertilize","spray","wait","debug","get_pos","get_map_size","get_current","get_tick"};
        l.allowedSyntax = {"expr","call","assign","if","for","in","range","def","while","else","not","and","or","break","continue"};
        l.allowedCrops = {"wheat","carrot","tomato","corn"};
        l.tutorialCode = u"# 用 break/continue 优化\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\ndef go_top():\n    while get_pos()[1] > 0:\n        move(\"up\")\n\n# 种三行玉米\nfor row in range(3):\n    for col in range(5):\n        cell = get_current()\n        if cell[\"state\"] != \"empty\":\n            if col < 4:\n                move(\"right\")\n            continue\n        till()\n        plant(\"corn\")\n        water()\n        fertilize()\n        if col < 4:\n            move(\"right\")\n    go_home()\n    if row < 2:\n        move(\"down\")\n\ngo_home()\ngo_top()\n\n# 智能巡逻（只处理有问题的格子）\ndef smart_patrol():\n    for row in range(3):\n        for col in range(5):\n            cell = get_current()\n            if not cell[\"hasBug\"] and cell[\"water\"] >= 0.15:\n                if col < 4:\n                    move(\"right\")\n                continue\n            if cell[\"hasBug\"]:\n                spray()\n            if cell[\"water\"] < 0.15:\n                water()\n            if col < 4:\n                move(\"right\")\n        go_home()\n        if row < 2:\n            move(\"down\")\n    go_home()\n    go_top()\n\n# 等待并巡逻\nfor t in range(15):\n    wait()\n    if t % 3 == 0:\n        smart_patrol()\n\n# 收割\ngo_home()\ngo_top()\nfor row in range(3):\n    for col in range(5):\n        cell = get_current()\n        if cell[\"state\"] == \"mature\":\n            harvest()\n        if col < 4:\n            move(\"right\")\n    go_home()\n    if row < 2:\n        move(\"down\")\n"_s;
        l.presetCells = {
            makeRock(4, 0), makeRock(4, 1), makeRock(4, 2), makeRock(4, 3),
            makePestZone(2, 0), makePestZone(4, 2)
        };
        l.goals = {
            {u"收获玉米 × 21"_s, GoalType::HarvestCount, "corn", 21, 0, false, StarTier::Star1},
            {u"180 秒内完成"_s, GoalType::TimeLimit, "", 180, 0, false, StarTier::Star2},
            {u"120 秒内完成"_s, GoalType::TimeLimit, "", 120, 0, false, StarTier::Star3}
        };
        levels_[13] = l;
    }

    // === 关卡 14：路径规划 (6×6) ===
    // 教学：return, get_goals(), 大地图路径规划
    {
        LevelConfig l;
        l.levelId = 14;
        l.name = u"路径规划"_s;
        l.description = u"在6×6大地图上规划高效耕种路径。目标：收获小麦×8、胡萝卜×8、番茄×8、玉米×7"_s;
        l.gridW = 6; l.gridH = 6;
        l.maxTimeSec = 300;

        l.bugProbability = 0.012f;
        l.allowedFunctions = {"move","till","plant","harvest","water","fertilize","spray","wait","debug","get_pos","get_map_size","get_current","get_tick","get_goals"};
        l.allowedSyntax = {"expr","call","assign","if","for","in","range","def","while","else","not","and","or","break","continue","return"};
        l.allowedCrops = {"wheat","carrot","tomato","corn"};
        l.tutorialCode = u"# 6×6 大地图路径规划\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\ndef go_top():\n    while get_pos()[1] > 0:\n        move(\"up\")\n\ndef plant_row(crop, count):\n    for i in range(count):\n        cell = get_current()\n        if cell[\"state\"] == \"empty\":\n            till()\n            plant(crop)\n            water()\n            if crop == \"carrot\":\n                water()\n            fertilize()\n        if i < count - 1:\n            move(\"right\")\n\ndef harvest_row(count):\n    for i in range(count):\n        cell = get_current()\n        if cell[\"state\"] == \"mature\":\n            harvest()\n        if i < count - 1:\n            move(\"right\")\n\ndef patrol_row(count):\n    for i in range(count):\n        cell = get_current()\n        if cell[\"hasBug\"]:\n            spray()\n        elif cell[\"water\"] < 0.15:\n            water()\n        if i < count - 1:\n            move(\"right\")\n\n# 种四行\nplant_row(\"wheat\", 6)\nmove(\"down\")\ngo_home()\nplant_row(\"carrot\", 6)\nmove(\"down\")\ngo_home()\nplant_row(\"tomato\", 6)\nmove(\"down\")\ngo_home()\nplant_row(\"corn\", 6)\n\n# 回到起点\ngo_home()\ngo_top()\n\n# 持续巡逻（每3 tick巡逻一次）\nfor t in range(20):\n    wait()\n    if t % 3 == 0:\n        for row in range(4):\n            patrol_row(6)\n            go_home()\n            if row < 3:\n                move(\"down\")\n        go_home()\n        go_top()\n\n# 收割\ngo_home()\ngo_top()\nfor row in range(4):\n    harvest_row(6)\n    if row < 3:\n        move(\"down\")\n        go_home()\n"_s;
        l.presetCells = {
            makeRock(5, 0), makeRock(5, 1), makeRock(5, 2), makeRock(5, 3), makeRock(3, 4),
            makePestZone(2, 1), makePestZone(5, 3)
        };
        l.goals = {
            {u"收获小麦 × 8"_s, GoalType::HarvestCount, "wheat", 8, 0, false, StarTier::Star1},
            {u"收获胡萝卜 × 8"_s, GoalType::HarvestCount, "carrot", 8, 0, false, StarTier::Star1},
            {u"收获番茄 × 8"_s, GoalType::HarvestCount, "tomato", 8, 0, false, StarTier::Star1},
            {u"收获玉米 × 7"_s, GoalType::HarvestCount, "corn", 7, 0, false, StarTier::Star1},
            {u"240 秒内完成"_s, GoalType::TimeLimit, "", 240, 0, false, StarTier::Star2},
            {u"150 秒内完成"_s, GoalType::TimeLimit, "", 150, 0, false, StarTier::Star3}
        };
        levels_[14] = l;
    }

    // =====================================================================
    // 阶段6：向日葵与高级耕种（关15-17）
    // =====================================================================

    // === 关卡 15：向日葵入门 (6×6) ===
    // 教学：向日葵的阳光加成和过涝致命
    {
        LevelConfig l;
        l.levelId = 15;
        l.name = u"向日葵入门"_s;
        l.description = u"向日葵为周围作物提供阳光加成，但过涝会致命。目标：收获番茄×33"_s;
        l.gridW = 6; l.gridH = 6;
        l.maxTimeSec = 300;

        l.bugProbability = 0.012f;
        l.allowedFunctions = {"move","till","plant","harvest","water","fertilize","spray","wait","debug","get_pos","get_map_size","get_current","get_tick","get_goals"};
        l.allowedSyntax = {"expr","call","assign","if","for","in","range","def","while","else","not","and","or","break","continue","return"};
        l.allowedCrops = {"wheat","carrot","tomato","corn","sunflower"};
        l.tutorialCode = u"# 向日葵为周围作物提供阳光加成\n# 向日葵不可收割，仅用于加速邻近作物\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\ndef go_top():\n    while get_pos()[1] > 0:\n        move(\"up\")\n\n# 布局：偶数格种向日葵，奇数格种番茄\nfor row in range(4):\n    for col in range(6):\n        cell = get_current()\n        if cell[\"state\"] == \"empty\":\n            till()\n            if col % 2 == 0:\n                plant(\"sunflower\")\n                water()  # 向日葵只浇一次\n            else:\n                plant(\"tomato\")\n                water()\n                fertilize()\n        if col < 5:\n            move(\"right\")\n    go_home()\n    if row < 3:\n        move(\"down\")\n\ngo_home()\ngo_top()\n\n# 等待并巡逻\nfor t in range(20):\n    wait()\n    if t % 5 == 0:\n        for row in range(4):\n            for col in range(6):\n                cell = get_current()\n                if cell[\"hasBug\"]:\n                    spray()\n                if col < 5:\n                    move(\"right\")\n            go_home()\n            if row < 3:\n                move(\"down\")\n        go_home()\n        go_top()\n\n# 收割（只收割可收割的作物）\ngo_home()\ngo_top()\nfor row in range(4):\n    for col in range(6):\n        cell = get_current()\n        if cell[\"state\"] == \"mature\":\n            harvest()\n        if col < 5:\n            move(\"right\")\n    go_home()\n    if row < 3:\n        move(\"down\")\n"_s;
        l.presetCells = {
            makeRock(5, 0), makeRock(5, 1), makeRock(5, 3),
            makePestZone(1, 2), makePestZone(4, 0)
        };
        l.goals = {
            {u"收获番茄 × 33"_s, GoalType::HarvestCount, "tomato", 33, 0, false, StarTier::Star1},
            {u"240 秒内完成"_s, GoalType::TimeLimit, "", 240, 0, false, StarTier::Star2},
            {u"180 秒内完成"_s, GoalType::TimeLimit, "", 180, 0, false, StarTier::Star3}
        };
        levels_[15] = l;
    }

    // === 关卡 16：阳光规划 (6×6) ===
    // 教学：向日葵布局优化
    {
        LevelConfig l;
        l.levelId = 16;
        l.name = u"阳光规划"_s;
        l.description = u"合理布局向日葵，最大化阳光加成覆盖。目标：收获玉米×31"_s;
        l.gridW = 6; l.gridH = 6;
        l.maxTimeSec = 300;

        l.bugProbability = 0.012f;
        l.allowedFunctions = {"move","till","plant","harvest","water","fertilize","spray","wait","debug","get_pos","get_map_size","get_current","get_tick","get_goals"};
        l.allowedSyntax = {"expr","call","assign","if","for","in","range","def","while","else","not","and","or","break","continue","return"};
        l.allowedCrops = {"wheat","carrot","tomato","corn","sunflower"};
        l.tutorialCode = u"# 向日葵棋盘格布局\n# 向日葵在偶数格，玉米在奇数格\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\ndef go_top():\n    while get_pos()[1] > 0:\n        move(\"up\")\n\n# 棋盘格布局\nfor row in range(5):\n    for col in range(6):\n        cell = get_current()\n        if cell[\"state\"] == \"empty\":\n            till()\n            if (row + col) % 2 == 0:\n                plant(\"sunflower\")\n                water()  # 向日葵只浇一次\n            else:\n                plant(\"corn\")\n                water()\n                fertilize()\n        if col < 5:\n            move(\"right\")\n    go_home()\n    if row < 4:\n        move(\"down\")\n\ngo_home()\ngo_top()\n\n# 持续巡逻（每4 tick巡逻一次）\nfor t in range(25):\n    wait()\n    if t % 4 == 0:\n        for row in range(5):\n            for col in range(6):\n                cell = get_current()\n                if cell[\"hasBug\"]:\n                    spray()\n                elif cell[\"water\"] < 0.15 and cell[\"crop\"] != \"sunflower\":\n                    water()\n                if col < 5:\n                    move(\"right\")\n            go_home()\n            if row < 4:\n                move(\"down\")\n        go_home()\n        go_top()\n\n# 收割\ngo_home()\ngo_top()\nfor row in range(5):\n    for col in range(6):\n        cell = get_current()\n        if cell[\"state\"] == \"mature\":\n            harvest()\n        if col < 5:\n            move(\"right\")\n    go_home()\n    if row < 4:\n        move(\"down\")\n"_s;
        l.presetCells = {
            makeRock(5, 0), makeRock(5, 1), makeRock(5, 2), makeRock(5, 3),
            makeRock(5, 4),
            makePestZone(2, 3), makePestZone(5, 0)
        };
        l.goals = {
            {u"收获玉米 × 31"_s, GoalType::HarvestCount, "corn", 31, 0, false, StarTier::Star1},
            {u"240 秒内完成"_s, GoalType::TimeLimit, "", 240, 0, false, StarTier::Star2},
            {u"180 秒内完成"_s, GoalType::TimeLimit, "", 180, 0, false, StarTier::Star3}
        };
        levels_[16] = l;
    }

    // === 关卡 17：效率挑战 (6×6) ===
    // 教学：限时高效耕种
    {
        LevelConfig l;
        l.levelId = 17;
        l.name = u"效率挑战"_s;
        l.description = u"限时内高效管理多种作物和向日葵。目标：收获小麦×15、玉米×15"_s;
        l.gridW = 6; l.gridH = 6;
        l.maxTimeSec = 240;

        l.bugProbability = 0.013f;
        l.allowedFunctions = {"move","till","plant","harvest","water","fertilize","spray","wait","debug","get_pos","get_map_size","get_current","get_tick","get_goals"};
        l.allowedSyntax = {"expr","call","assign","if","for","in","range","def","while","else","not","and","or","break","continue","return"};
        l.allowedCrops = {"wheat","carrot","tomato","corn","sunflower"};
        l.tutorialCode = u"# 限时挑战！高效耕种\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\ndef go_top():\n    while get_pos()[1] > 0:\n        move(\"up\")\n\n# 高效种植（检查空地）\nfor row in range(6):\n    for col in range(6):\n        cell = get_current()\n        if cell[\"state\"] == \"empty\":\n            till()\n            if (row + col) % 3 == 0:\n                plant(\"sunflower\")\n                water()\n            elif row < 3:\n                plant(\"wheat\")\n                water()\n                fertilize()\n            else:\n                plant(\"corn\")\n                water()\n                fertilize()\n        if col < 5:\n            move(\"right\")\n    go_home()\n    if row < 5:\n        move(\"down\")\n\ngo_home()\ngo_top()\n\n# 快速巡逻\nfor t in range(15):\n    wait()\n    if t % 3 == 0:\n        for row in range(6):\n            for col in range(6):\n                cell = get_current()\n                if cell[\"hasBug\"]:\n                    spray()\n                if col < 5:\n                    move(\"right\")\n            go_home()\n            if row < 5:\n                move(\"down\")\n        go_home()\n        go_top()\n\n# 收割\ngo_home()\ngo_top()\nfor row in range(6):\n    for col in range(6):\n        cell = get_current()\n        if cell[\"state\"] == \"mature\":\n            harvest()\n        if col < 5:\n            move(\"right\")\n    go_home()\n    if row < 5:\n        move(\"down\")\n"_s;
        l.presetCells = {
            makeRock(5, 0), makeRock(5, 1), makeRock(5, 2), makeRock(5, 3),
            makeRock(5, 4), makeRock(5, 5),
            makePestZone(0, 2), makePestZone(3, 3), makePestZone(5, 5)
        };
        l.goals = {
            {u"收获小麦 × 15"_s, GoalType::HarvestCount, "wheat", 15, 0, false, StarTier::Star1},
            {u"收获玉米 × 15"_s, GoalType::HarvestCount, "corn", 15, 0, false, StarTier::Star1},
            {u"180 秒内完成"_s, GoalType::TimeLimit, "", 180, 0, false, StarTier::Star2},
            {u"120 秒内完成"_s, GoalType::TimeLimit, "", 120, 0, false, StarTier::Star3}
        };
        levels_[17] = l;
    }

    // =====================================================================
    // 阶段7：综合应用（关18-19）
    // =====================================================================

    // === 关卡 18：大农场 (7×7) ===
    // 教学：大规模综合管理
    {
        LevelConfig l;
        l.levelId = 18;
        l.name = u"大农场"_s;
        l.description = u"7×7大农场，综合运用所有技能。目标：收获小麦×10、胡萝卜×10、番茄×10、玉米×10"_s;
        l.gridW = 7; l.gridH = 7;
        l.maxTimeSec = 360;

        l.bugProbability = 0.014f;
        l.allowedFunctions = {"move","till","plant","harvest","water","fertilize","spray","wait","debug","get_pos","get_map_size","get_current","get_tick","get_goals"};
        l.allowedSyntax = {"expr","call","assign","if","for","in","range","def","while","else","not","and","or","break","continue","return"};
        l.allowedCrops = {"wheat","carrot","tomato","corn","sunflower"};
        l.tutorialCode = u"# 7×7 大农场\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\ndef go_top():\n    while get_pos()[1] > 0:\n        move(\"up\")\n\n# 种植所有作物（检查空地）\ndef plant_smart(crop):\n    for i in range(7):\n        cell = get_current()\n        if cell[\"state\"] == \"empty\":\n            till()\n            plant(crop)\n            water()\n            if crop == \"carrot\":\n                water()\n            if crop != \"sunflower\":\n                fertilize()\n        if i < 6:\n            move(\"right\")\n\nplant_smart(\"sunflower\")\nmove(\"down\")\ngo_home()\nplant_smart(\"wheat\")\nmove(\"down\")\ngo_home()\nplant_smart(\"carrot\")\nmove(\"down\")\ngo_home()\nplant_smart(\"tomato\")\nmove(\"down\")\ngo_home()\nplant_smart(\"corn\")\n\n# 回到起点\ngo_home()\ngo_top()\n\n# 持续巡逻\nfor t in range(20):\n    wait()\n    if t % 3 == 0:\n        for row in range(5):\n            for col in range(7):\n                cell = get_current()\n                if cell[\"hasBug\"]:\n                    spray()\n                elif cell[\"water\"] < 0.1 and cell[\"crop\"] != \"sunflower\":\n                    water()\n                if col < 6:\n                    move(\"right\")\n            go_home()\n            if row < 4:\n                move(\"down\")\n        go_home()\n        go_top()\n\n# 收割\ngo_home()\ngo_top()\nfor row in range(5):\n    for col in range(7):\n        cell = get_current()\n        if cell[\"state\"] == \"mature\":\n            harvest()\n        if col < 6:\n            move(\"right\")\n    go_home()\n    if row < 4:\n        move(\"down\")\n"_s;
        l.presetCells = {
            makeRock(6, 0), makeRock(6, 1), makeRock(6, 2), makeRock(6, 3),
            makeRock(6, 4), makeRock(5, 5),
            makeRock(1, 6), makeRock(4, 6),
            makePestZone(2, 1), makePestZone(5, 2), makePestZone(0, 5)
        };
        l.goals = {
            {u"收获小麦 × 10"_s, GoalType::HarvestCount, "wheat", 10, 0, false, StarTier::Star1},
            {u"收获胡萝卜 × 10"_s, GoalType::HarvestCount, "carrot", 10, 0, false, StarTier::Star1},
            {u"收获番茄 × 10"_s, GoalType::HarvestCount, "tomato", 10, 0, false, StarTier::Star1},
            {u"收获玉米 × 10"_s, GoalType::HarvestCount, "corn", 10, 0, false, StarTier::Star1},
            {u"270 秒内完成"_s, GoalType::TimeLimit, "", 270, 0, false, StarTier::Star2},
            {u"200 秒内完成"_s, GoalType::TimeLimit, "", 200, 0, false, StarTier::Star3}
        };
        levels_[18] = l;
    }

    // === 关卡 19：极限耕种 (7×7) ===
    // 教学：极限效率挑战
    {
        LevelConfig l;
        l.levelId = 19;
        l.name = u"极限耕种"_s;
        l.description = u"7×7大农场限时挑战，极限效率。目标：收获玉米×42"_s;
        l.gridW = 7; l.gridH = 7;
        l.maxTimeSec = 300;

        l.bugProbability = 0.015f;
        l.allowedFunctions = {"move","till","plant","harvest","water","fertilize","spray","wait","debug","get_pos","get_map_size","get_current","get_tick","get_goals"};
        l.allowedSyntax = {"expr","call","assign","if","for","in","range","def","while","else","not","and","or","break","continue","return"};
        l.allowedCrops = {"wheat","carrot","tomato","corn","sunflower"};
        l.tutorialCode = u"# 极限耕种挑战！\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\ndef go_top():\n    while get_pos()[1] > 0:\n        move(\"up\")\n\n# 棋盘格布局：向日葵+玉米\nfor row in range(7):\n    for col in range(7):\n        cell = get_current()\n        if cell[\"state\"] == \"empty\":\n            till()\n            if (row + col) % 2 == 0:\n                plant(\"sunflower\")\n                water()\n            else:\n                plant(\"corn\")\n                water()\n                fertilize()\n        if col < 6:\n            move(\"right\")\n    go_home()\n    if row < 6:\n        move(\"down\")\n\ngo_home()\ngo_top()\n\n# 高效巡逻\nfor t in range(20):\n    wait()\n    if t % 2 == 0:\n        for row in range(7):\n            for col in range(7):\n                cell = get_current()\n                if cell[\"hasBug\"]:\n                    spray()\n                elif cell[\"water\"] < 0.1 and cell[\"crop\"] != \"sunflower\":\n                    water()\n                elif cell[\"state\"] == \"mature\":\n                    harvest()\n                if col < 6:\n                    move(\"right\")\n            go_home()\n            if row < 6:\n                move(\"down\")\n        go_home()\n        go_top()\n\n# 收割\ngo_home()\ngo_top()\nfor row in range(7):\n    for col in range(7):\n        cell = get_current()\n        if cell[\"state\"] == \"mature\":\n            harvest()\n        if col < 6:\n            move(\"right\")\n    go_home()\n    if row < 6:\n        move(\"down\")\n"_s;
        l.presetCells = {
            makeRock(6, 0), makeRock(6, 1), makeRock(6, 2), makeRock(6, 3),
            makeRock(6, 4), makeRock(6, 5), makeRock(6, 6),
            makePestZone(2, 0), makePestZone(5, 1), makePestZone(0, 4), makePestZone(4, 6)
        };
        l.goals = {
            {u"收获玉米 × 42"_s, GoalType::HarvestCount, "corn", 42, 0, false, StarTier::Star1},
            {u"240 秒内完成"_s, GoalType::TimeLimit, "", 240, 0, false, StarTier::Star2},
            {u"180 秒内完成"_s, GoalType::TimeLimit, "", 180, 0, false, StarTier::Star3}
        };
        levels_[19] = l;
    }

    // =====================================================================
    // 阶段8：终极挑战（关20）
    // =====================================================================

    // === 关卡 20：终极农场 (8×8) ===
    // 教学：全 API + 全作物 + 全机制
    {
        LevelConfig l;
        l.levelId = 20;
        l.name = u"终极农场"_s;
        l.description = u"8×8 终极农场，综合运用所有技能。目标：收获小麦×14、胡萝卜×14、番茄×14、玉米×14"_s;
        l.gridW = 8; l.gridH = 8;
        l.maxTimeSec = 420;

        l.bugProbability = 0.018f;
        l.allowedFunctions = {"move","till","plant","harvest","water","fertilize","spray","wait","debug","get_pos","get_map_size","get_current","get_tick","get_goals"};
        l.allowedSyntax = {"expr","call","assign","if","for","in","range","def","while","else","not","and","or","break","continue","return"};
        l.allowedCrops = {"wheat","carrot","tomato","corn","sunflower"};
        l.tutorialCode = u"# 终极农场！全 API + 全机制\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\ndef go_top():\n    while get_pos()[1] > 0:\n        move(\"up\")\n\n# 棋盘格布局（检查空地）\nfor row in range(8):\n    for col in range(8):\n        cell = get_current()\n        if cell[\"state\"] == \"empty\":\n            till()\n            if (row + col) % 2 == 0:\n                plant(\"sunflower\")\n                water()  # 向日葵只浇一次\n            elif row < 3:\n                plant(\"wheat\")\n                water()\n                fertilize()\n            elif row < 5:\n                plant(\"carrot\")\n                water()\n                water()\n                fertilize()\n            else:\n                plant(\"corn\")\n                water()\n                fertilize()\n        if col < 7:\n            move(\"right\")\n    go_home()\n    if row < 7:\n        move(\"down\")\n\ngo_home()\ngo_top()\n\n# 智能巡逻（最优化）\ndef patrol():\n    for row in range(8):\n        for col in range(8):\n            cell = get_current()\n            if cell[\"hasBug\"]:\n                spray()\n            elif cell[\"state\"] == \"mature\":\n                harvest()\n            elif cell[\"water\"] < 0.1 and cell[\"crop\"] != \"sunflower\":\n                water()\n            if col < 7:\n                move(\"right\")\n        go_home()\n        if row < 7:\n            move(\"down\")\n    go_home()\n    go_top()\n\n# 持续巡逻\nfor t in range(20):\n    wait()\n    if t % 2 == 0:\n        patrol()\n\n# 最终收割\ngo_home()\ngo_top()\nfor row in range(8):\n    for col in range(8):\n        cell = get_current()\n        if cell[\"state\"] == \"mature\":\n            harvest()\n        if col < 7:\n            move(\"right\")\n    go_home()\n    if row < 7:\n        move(\"down\")\n"_s;
        l.presetCells = {
            makeRock(7, 0), makeRock(7, 1), makeRock(7, 2), makeRock(7, 3),
            makeRock(7, 4), makeRock(7, 5), makeRock(7, 6), makeRock(7, 7),
            makePestZone(3, 1), makePestZone(6, 2), makePestZone(0, 5),
            makePestZone(5, 6)
        };
        l.goals = {
            {u"收获小麦 × 14"_s, GoalType::HarvestCount, "wheat", 14, 0, false, StarTier::Star1},
            {u"收获胡萝卜 × 14"_s, GoalType::HarvestCount, "carrot", 14, 0, false, StarTier::Star1},
            {u"收获番茄 × 14"_s, GoalType::HarvestCount, "tomato", 14, 0, false, StarTier::Star1},
            {u"收获玉米 × 14"_s, GoalType::HarvestCount, "corn", 14, 0, false, StarTier::Star1},
            {u"330 秒内完成"_s, GoalType::TimeLimit, "", 330, 0, false, StarTier::Star2},
            {u"240 秒内完成"_s, GoalType::TimeLimit, "", 240, 0, false, StarTier::Star3}
        };
        levels_[20] = l;
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
        {"tutorialCode", l.tutorialCode},
        {"gridW", l.gridW}, {"gridH", l.gridH},
        {"maxTimeSec", l.maxTimeSec},
        {"goals", goals},
        {"allowedFunctions", allowedFunctions}
    };
}

QVariantMap LevelManager::getLevelNewContent(int id) const {
    if (!levels_.contains(id)) return {};

    const auto &curr = levels_[id];

    // Get previous level's allowed sets
    QSet<QString> prevFunctions;
    QSet<QString> prevSyntax;
    QSet<QString> prevCrops;
    if (id > 1 && levels_.contains(id - 1)) {
        const auto &prev = levels_[id - 1];
        prevFunctions = prev.allowedFunctions;
        prevSyntax = prev.allowedSyntax;
        prevCrops = prev.allowedCrops;
    }

    // Compute new items
    QVariantList newFunctions;
    QStringList newFuncList;
    for (const auto &f : curr.allowedFunctions) {
        if (!prevFunctions.contains(f)) {
            newFuncList.append(f);
        }
    }
    newFuncList.sort();
    for (const auto &f : newFuncList) {
        newFunctions.append(f);
    }

    QVariantList newSyntax;
    // Only show real Python keywords, not meta-tags like "expr"/"call"/"assign"
    const QSet<QString> displayableKeywords = {
        "for","in","range","if","else","while","def",
        "break","continue","return","and","or","not"
    };
    QStringList newSyntaxList;
    for (const auto &s : curr.allowedSyntax) {
        if (!prevSyntax.contains(s) && displayableKeywords.contains(s)) {
            newSyntaxList.append(s);
        }
    }
    newSyntaxList.sort();
    for (const auto &s : newSyntaxList) {
        newSyntax.append(s);
    }

    QVariantList newCrops;
    QStringList newCropList;
    for (const auto &c : curr.allowedCrops) {
        if (!prevCrops.contains(c)) {
            newCropList.append(c);
        }
    }
    newCropList.sort();
    for (const auto &c : newCropList) {
        newCrops.append(c);
    }

    return {
        {"newFunctions", newFunctions},
        {"newSyntax", newSyntax},
        {"newCrops", newCrops}
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

void LevelManager::resetProgress() {
    progress_.clear();
    progress_[1].unlocked = true;
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
