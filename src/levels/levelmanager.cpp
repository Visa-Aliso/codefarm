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
    // 阶段1：入门耕种（关1-3）— 顺序 / 变量条件 / for循环
    // =====================================================================

    // === 关卡 1：破土 (1×1) ===
    // 教学：顺序调用、注释
    {
        LevelConfig l;
        l.levelId = 1;
        l.name = u"破土"_s;
        l.description = u"顺序执行是编程的基础：犁地→种植→浇水→等待→收割。目标：收获小麦×1"_s;
        l.gridW = 1; l.gridH = 1;
        l.maxTimeSec = 300;

        l.bugProbability = 0.0f;
        l.allowedFunctions = {"till","plant","water","wait","harvest"};
        l.allowedSyntax = {};
        l.allowedCrops = {"wheat"};
        l.allowedBuiltins = {};
        l.star2TickThreshold = 12;
        l.star3TickThreshold = 10;
        l.star3RequiredFeatures = {};
        l.tutorialCode = u"# 顺序执行是编程的基础\n# # 后面是注释，机器不执行\n\ntill()            # 1. 犁地\nplant(\"wheat\")    # 2. 种小麦\nwater()           # 3. 浇水\n# 小麦需 6 tick 成熟——重复写 6 次 wait\n# 下一关学循环就能消除这种重复！\nwait()\nwait()\nwait()\nwait()\nwait()\nwait()\nharvest()         # 4. 收割\n"_s;
        l.presetCells = {};
        l.goals = {
            {u"收获小麦 × 1"_s, GoalType::HarvestCount, "wheat", 1, 0, false, StarTier::Star1},
            {u"效率 ≤ 15 tick"_s, GoalType::TickLimit, "", 15, 0, false, StarTier::Star2},
            {u"效率 ≤ 10 tick"_s, GoalType::TickLimit, "", 10, 0, false, StarTier::Star3}
        };
        levels_[1] = l;
    }

    // === 关卡 2：迈步 (2×1) ===
    // 教学：变量赋值、if 条件
    {
        LevelConfig l;
        l.levelId = 2;
        l.name = u"迈步"_s;
        l.description = u"用变量记录宽度，用 if 判断是否需要回退。目标：收获小麦×2"_s;
        l.gridW = 2; l.gridH = 1;
        l.maxTimeSec = 300;

        l.bugProbability = 0.0f;
        l.allowedFunctions = {"move","till","plant","water","wait","harvest","get_pos"};
        l.allowedSyntax = {"assign","if"};
        l.allowedCrops = {"wheat"};
        l.allowedBuiltins = {};
        l.star2TickThreshold = 22;
        l.star3TickThreshold = 18;
        l.star3RequiredFeatures = {"assign","if"};
        l.tutorialCode = u"# 变量赋值：用 = 把值存起来\ncrop = \"wheat\"\n\ntill()\nplant(crop)\nwater()\nmove(\"right\")\ntill()\nplant(crop)\nwater()\n\n# 等待成熟\nwait()\nwait()\nwait()\nwait()\nwait()\nwait()\n\nharvest()\n# if 判断：是否需要回到左边那格\nif get_pos()[0] != 0:\n    move(\"left\")\nharvest()\n"_s;
        l.presetCells = {};
        l.goals = {
            {u"收获小麦 × 2"_s, GoalType::HarvestCount, "wheat", 2, 0, false, StarTier::Star1},
            {u"效率 ≤ 25 tick"_s, GoalType::TickLimit, "", 25, 0, false, StarTier::Star2},
            {u"使用新特性: 变量赋值, if 条件"_s, GoalType::Custom, "", 1, 0, false, StarTier::Star3}
        };
        levels_[2] = l;
    }

    // === 关卡 3：田字 (2×2) ===
    // 教学：for + range 消除重复
    {
        LevelConfig l;
        l.levelId = 3;
        l.name = u"田字"_s;
        l.description = u"用 for 循环在 2×2 网格上批量耕种。目标：收获小麦×4"_s;
        l.gridW = 2; l.gridH = 2;
        l.maxTimeSec = 300;

        l.bugProbability = 0.0f;
        l.allowedFunctions = {"move","till","plant","water","wait","harvest","get_pos","get_map_size"};
        l.allowedSyntax = {"assign","if","for"};
        l.allowedCrops = {"wheat"};
        l.allowedBuiltins = {"range"};
        l.star2TickThreshold = 50;
        l.star3TickThreshold = 42;
        l.star3RequiredFeatures = {"for"};
        l.tutorialCode = u"# for + range 消除重复！range(2) 生成 0,1\n\n# 种植阶段：外层行，内层列\nfor row in range(2):\n    for col in range(2):\n        till()\n        plant(\"wheat\")\n        water()\n        if col < 1:\n            move(\"right\")\n    # 回到行首\n    move(\"left\")\n    if row < 1:\n        move(\"down\")\n\n# 回到起点 (0,0)\nmove(\"up\")\n\n# 等待成熟\nfor i in range(6):\n    wait()\n\n# 收割阶段\nfor row in range(2):\n    for col in range(2):\n        harvest()\n        if col < 1:\n            move(\"right\")\n    move(\"left\")\n    if row < 1:\n        move(\"down\")\n"_s;
        l.presetCells = {};
        l.goals = {
            {u"收获小麦 × 4"_s, GoalType::HarvestCount, "wheat", 4, 0, false, StarTier::Star1},
            {u"效率 ≤ 45 tick"_s, GoalType::TickLimit, "", 45, 0, false, StarTier::Star2},
            {u"使用新特性: for 循环"_s, GoalType::Custom, "", 1, 0, false, StarTier::Star3}
        };
        levels_[3] = l;
    }

    // =====================================================================
    // 阶段2：基础循环与函数（关4-6）— while / def / return
    // =====================================================================

    // === 关卡 4：绕岩 (3×3) ===
    // 教学：while 循环、get_current 查询字典
    {
        LevelConfig l;
        l.levelId = 4;
        l.name = u"绕岩"_s;
        l.description = u"用 while 循环回到原点，用 get_current() 检查格子状态。岩石格需跳过。目标：收获小麦×8"_s;
        l.gridW = 3; l.gridH = 3;
        l.maxTimeSec = 300;

        l.bugProbability = 0.0f;
        l.allowedFunctions = {"move","till","plant","water","wait","harvest","get_pos","get_map_size","get_current"};
        l.allowedSyntax = {"assign","if","for","while","def"};
        l.allowedCrops = {"wheat"};
        l.allowedBuiltins = {"range"};
        l.star2TickThreshold = 75;
        l.star3TickThreshold = 64;
        l.star3RequiredFeatures = {"while"};
        l.tutorialCode = u"# while：条件为真就一直循环\n# get_current() 返回当前格信息（字典），用 [\"键\"] 取值\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\ndef go_top():\n    while get_pos()[1] > 0:\n        move(\"up\")\n\nfor row in range(3):\n    for col in range(3):\n        if get_current()[\"state\"] == \"empty\":\n            till()\n            plant(\"wheat\")\n            water()\n        if col < 2:\n            move(\"right\")\n    go_home()\n    if row < 2:\n        move(\"down\")\n\ngo_home()\ngo_top()\n\nfor i in range(6):\n    wait()\n\nfor row in range(3):\n    for col in range(3):\n        if get_current()[\"state\"] == \"mature\":\n            harvest()\n        if col < 2:\n            move(\"right\")\n    go_home()\n    if row < 2:\n        move(\"down\")\n"_s;
        l.presetCells = {makeRock(2, 0)};
        l.goals = {
            {u"收获小麦 × 8"_s, GoalType::HarvestCount, "wheat", 8, 0, false, StarTier::Star1},
            {u"效率 ≤ 90 tick"_s, GoalType::TickLimit, "", 90, 0, false, StarTier::Star2},
            {u"使用新特性: while 循环"_s, GoalType::Custom, "", 1, 0, false, StarTier::Star3}
        };
        levels_[4] = l;
    }

    // === 关卡 5：封装 (3×3) ===
    // 教学：def 函数定义、参数；胡萝卜过涝
    {
        LevelConfig l;
        l.levelId = 5;
        l.name = u"封装"_s;
        l.description = u"用 def 封装重复流程。胡萝卜需两次水（三次就过涝）。目标：收获小麦×3、胡萝卜×5"_s;
        l.gridW = 3; l.gridH = 3;
        l.maxTimeSec = 300;

        l.bugProbability = 0.0f;
        l.allowedFunctions = {"move","till","plant","water","wait","harvest","get_pos","get_map_size","get_current"};
        l.allowedSyntax = {"assign","if","for","while","def"};
        l.allowedCrops = {"wheat","carrot"};
        l.allowedBuiltins = {"range"};
        l.star2TickThreshold = 85;
        l.star3TickThreshold = 77;
        l.star3RequiredFeatures = {"def"};
        l.tutorialCode = u"# def 把重复流程封装成函数，可带参数\n\ndef plant_cell(crop):\n    till()\n    plant(crop)\n    water()\n    if crop == \"carrot\":\n        water()        # 胡萝卜要两次水\n\ndef plant_row(crop):\n    for col in range(3):\n        if get_current()[\"state\"] == \"empty\":\n            plant_cell(crop)\n        if col < 2:\n            move(\"right\")\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\ndef go_top():\n    while get_pos()[1] > 0:\n        move(\"up\")\n\nfor row in range(3):\n    if row == 0:\n        plant_row(\"wheat\")\n    if row != 0:\n        plant_row(\"carrot\")\n    go_home()\n    if row < 2:\n        move(\"down\")\n\ngo_home()\ngo_top()\n\nfor i in range(14):     # 胡萝卜需 14 tick\n    wait()\n\nfor row in range(3):\n    for col in range(3):\n        if get_current()[\"state\"] == \"mature\":\n            harvest()\n        if col < 2:\n            move(\"right\")\n    go_home()\n    if row < 2:\n        move(\"down\")\n"_s;
        l.presetCells = {makeRock(2, 2)};
        l.goals = {
            {u"收获小麦 × 3"_s, GoalType::HarvestCount, "wheat", 3, 0, false, StarTier::Star1},
            {u"收获胡萝卜 × 5"_s, GoalType::HarvestCount, "carrot", 5, 0, false, StarTier::Star1},
            {u"效率 ≤ 110 tick"_s, GoalType::TickLimit, "", 110, 0, false, StarTier::Star2},
            {u"使用新特性: def 函数定义"_s, GoalType::Custom, "", 1, 0, false, StarTier::Star3}
        };
        levels_[5] = l;
    }

    // === 关卡 6：返回 (3×3) ===
    // 教学：return 返回值、elif、not 取反
    {
        LevelConfig l;
        l.levelId = 6;
        l.name = u"返回"_s;
        l.description = u"用 return 让函数返回值，用 elif 处理多分支，用 not 取反。目标：收获胡萝卜×9"_s;
        l.gridW = 3; l.gridH = 3;
        l.maxTimeSec = 300;

        l.bugProbability = 0.0f;
        l.allowedFunctions = {"move","till","plant","water","wait","harvest","get_pos","get_map_size","get_current","debug"};
        l.allowedSyntax = {"assign","if","else","for","while","def","return","not"};
        l.allowedCrops = {"wheat","carrot"};
        l.allowedBuiltins = {"range"};
        l.star2TickThreshold = 100;
        l.star3TickThreshold = 87;
        l.star3RequiredFeatures = {"return","not"};
        l.tutorialCode = u"# return 让函数返回一个值；elif/else 处理多分支；not 取反\n\ndef is_mature():\n    s = get_current()[\"state\"]\n    if s == \"mature\":\n        return True\n    elif s == \"empty\":\n        return False\n    else:\n        return False\n\ndef plant_row(crop):\n    for col in range(3):\n        if get_current()[\"state\"] == \"empty\":\n            till()\n            plant(crop)\n            water()\n            water()\n        if col < 2:\n            move(\"right\")\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\ndef go_top():\n    while get_pos()[1] > 0:\n        move(\"up\")\n\nfor row in range(3):\n    plant_row(\"carrot\")\n    go_home()\n    if row < 2:\n        move(\"down\")\n\ngo_home()\ngo_top()\n\nfor i in range(14):\n    wait()\n\n# 收割：用 not 判断未成熟，else 收割\nfor row in range(3):\n    for col in range(3):\n        if not is_mature():\n            pass  # 未成熟，跳过\n        else:\n            harvest()\n        if col < 2:\n            move(\"right\")\n    go_home()\n    if row < 2:\n        move(\"down\")\n"_s;
        l.presetCells = {};
        l.goals = {
            {u"收获胡萝卜 × 9"_s, GoalType::HarvestCount, "carrot", 9, 0, false, StarTier::Star1},
            {u"效率 ≤ 120 tick"_s, GoalType::TickLimit, "", 120, 0, false, StarTier::Star2},
            {u"使用新特性: return, not 取反"_s, GoalType::Custom, "", 1, 0, false, StarTier::Star3}
        };
        levels_[6] = l;
    }

    // =====================================================================
    // 阶段3：进阶管理（关7-9）— 布尔 / break / 除虫 / 玉米
    // =====================================================================

    // === 关卡 7：施肥 (4×4) ===
    // 教学：and/or 组合条件、fertilize、番茄
    {
        LevelConfig l;
        l.levelId = 7;
        l.name = u"施肥"_s;
        l.description = u"施肥加速 1.5-1.8 倍。用 and/or 组合多个条件。目标：收获小麦×3、胡萝卜×4、番茄×7"_s;
        l.gridW = 4; l.gridH = 4;
        l.maxTimeSec = 300;

        l.bugProbability = 0.0f;
        l.allowedFunctions = {"move","till","plant","water","fertilize","wait","harvest","get_pos","get_map_size","get_current"};
        l.allowedSyntax = {"assign","if","else","for","while","def","return","and","or","not"};
        l.allowedCrops = {"wheat","carrot","tomato"};
        l.allowedBuiltins = {"range"};
        l.star2TickThreshold = 160;
        l.star3TickThreshold = 141;
        l.star3RequiredFeatures = {"and","or","fertilize"};
        l.tutorialCode = u"# fertilize 施肥加速；and/or 组合多个条件\n\ndef plant_cell(crop):\n    till()\n    plant(crop)\n    water()\n    if crop == \"carrot\" or crop == \"tomato\":   # or\n        water()\n    fertilize()\n\ndef plant_row(crop):\n    for col in range(4):\n        if get_current()[\"state\"] == \"empty\":\n            plant_cell(crop)\n        if col < 3:\n            move(\"right\")\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\ndef go_top():\n    while get_pos()[1] > 0:\n        move(\"up\")\n\nfor row in range(4):\n    if row == 0:\n        plant_row(\"wheat\")\n    elif row == 1:\n        plant_row(\"carrot\")\n    else:\n        plant_row(\"tomato\")\n    go_home()\n    if row < 3:\n        move(\"down\")\n\ngo_home()\ngo_top()\n\nfor i in range(10):\n    wait()\n\nfor row in range(4):\n    for col in range(4):\n        c = get_current()\n        if c[\"state\"] == \"mature\" and c[\"crop\"] != \"sunflower\":  # and\n            harvest()\n        if col < 3:\n            move(\"right\")\n    go_home()\n    if row < 3:\n        move(\"down\")\n"_s;
        l.presetCells = {makeRock(3, 0), makeRock(3, 2)};
        l.goals = {
            {u"收获小麦 × 3"_s, GoalType::HarvestCount, "wheat", 3, 0, false, StarTier::Star1},
            {u"收获胡萝卜 × 4"_s, GoalType::HarvestCount, "carrot", 4, 0, false, StarTier::Star1},
            {u"收获番茄 × 7"_s, GoalType::HarvestCount, "tomato", 7, 0, false, StarTier::Star1},
            {u"效率 ≤ 200 tick"_s, GoalType::TickLimit, "", 200, 0, false, StarTier::Star2},
            {u"使用新特性: and/or, fertilize"_s, GoalType::Custom, "", 1, 0, false, StarTier::Star3}
        };
        levels_[7] = l;
    }

    // === 关卡 8：除虫 (4×4) ===
    // 教学：break/continue、spray、虫害区
    {
        LevelConfig l;
        l.levelId = 8;
        l.name = u"除虫"_s;
        l.description = u"用 break 跳出循环、continue 跳过本次。spray() 杀虫。目标：收获番茄×14"_s;
        l.gridW = 4; l.gridH = 4;
        l.maxTimeSec = 300;

        l.bugProbability = 0.003f;
        l.allowedFunctions = {"move","till","plant","water","fertilize","spray","wait","harvest","get_pos","get_map_size","get_current"};
        l.allowedSyntax = {"assign","if","else","for","while","def","return","and","or","not","break","continue"};
        l.allowedCrops = {"wheat","carrot","tomato"};
        l.allowedBuiltins = {"range"};
        l.star2TickThreshold = 320;
        l.star3TickThreshold = 285;
        l.star3RequiredFeatures = {"break","continue","spray"};
        l.tutorialCode = u"# spray 除虫；continue 跳过本次剩余；break 跳出整个循环\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\ndef go_top():\n    while get_pos()[1] > 0:\n        move(\"up\")\n\nfor row in range(4):\n    for col in range(4):\n        if get_current()[\"state\"] == \"empty\":\n            till()\n            plant(\"tomato\")\n            water()\n            fertilize()\n        if col < 3:\n            move(\"right\")\n    go_home()\n    if row < 3:\n        move(\"down\")\n\ngo_home()\ngo_top()\n\n# break 演示：移动到最后几行，等那里的番茄成熟就停止\nfor _ in range(3):\n    move(\"down\")\nfor i in range(20):\n    if get_current()[\"state\"] == \"mature\":\n        break\n    wait()\ngo_home()\ngo_top()\n\n# continue 演示：巡逻时没虫的格子直接跳过\ndef patrol():\n    for row in range(4):\n        for col in range(4):\n            cell = get_current()\n            if not cell[\"hasBug\"]:\n                if col < 3:\n                    move(\"right\")\n                continue              # 跳过处理，进入下一格\n            spray()\n            if col < 3:\n                move(\"right\")\n        go_home()\n        if row < 3:\n            move(\"down\")\n    go_home()\n    go_top()\n\nfor t in range(15):\n    wait()\n    if t % 3 == 0:\n        patrol()\n\ngo_home()\ngo_top()\nfor row in range(4):\n    for col in range(4):\n        if get_current()[\"state\"] == \"mature\":\n            harvest()\n        if col < 3:\n            move(\"right\")\n    go_home()\n    if row < 3:\n        move(\"down\")\n"_s;
        l.presetCells = {makeRock(3, 3), makePestZone(2, 1)};
        l.goals = {
            {u"收获番茄 × 14"_s, GoalType::HarvestCount, "tomato", 14, 0, false, StarTier::Star1},
            {u"效率 ≤ 400 tick"_s, GoalType::TickLimit, "", 400, 0, false, StarTier::Star2},
            {u"使用新特性: break/continue, spray"_s, GoalType::Custom, "", 1, 0, false, StarTier::Star3}
        };
        levels_[8] = l;
    }

    // === 关卡 9：玉米 (5×5) ===
    // 巩固关：玉米抗旱、综合巡逻（无新语法/函数/内置）
    {
        LevelConfig l;
        l.levelId = 9;
        l.name = u"玉米"_s;
        l.description = u"玉米生长慢且缺水会掉进度，需要持续供水。综合运用所学。目标：收获玉米×21"_s;
        l.gridW = 5; l.gridH = 5;
        l.maxTimeSec = 300;

        l.bugProbability = 0.005f;
        l.allowedFunctions = {"move","till","plant","water","fertilize","spray","wait","harvest","get_pos","get_map_size","get_current","debug"};
        l.allowedSyntax = {"assign","if","else","for","while","def","return","and","or","not","break","continue"};
        l.allowedCrops = {"wheat","carrot","tomato","corn"};
        l.allowedBuiltins = {"range"};
        l.star2TickThreshold = 135;
        l.star3TickThreshold = 115;
        l.star3RequiredFeatures = {};   // 巩固关：★3 退化为纯 tick 门槛
        l.tutorialCode = u"# 玉米抗旱：缺水会掉进度，需持续供水\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\ndef go_top():\n    while get_pos()[1] > 0:\n        move(\"up\")\n\n# 种满所有行玉米并施肥\nfor row in range(5):\n    for col in range(5):\n        if get_current()[\"state\"] == \"empty\":\n            till()\n            plant(\"corn\")\n            water()\n            water()\n            fertilize()\n        if col < 4:\n            move(\"right\")\n    go_home()\n    if row < 4:\n        move(\"down\")\n\ngo_home()\ngo_top()\n\n# 巡逻防虫补水并收割成熟作物\nfor t in range(22):\n    wait()\n    if t % 3 == 0:\n        for row in range(5):\n            for col in range(5):\n                cell = get_current()\n                if cell[\"hasBug\"]:\n                    spray()\n                elif cell[\"state\"] == \"mature\":\n                    harvest()\n                elif cell[\"water\"] < 0.25:\n                    water()\n                if col < 4:\n                    move(\"right\")\n            go_home()\n            if row < 4:\n                move(\"down\")\n        go_home()\n        go_top()\n\ngo_home()\ngo_top()\nfor row in range(5):\n    for col in range(5):\n        if get_current()[\"state\"] == \"mature\":\n            harvest()\n        if col < 4:\n            move(\"right\")\n    go_home()\n    if row < 4:\n        move(\"down\")\n"_s;
        l.presetCells = {makeRock(4, 1), makeRock(4, 4), makePestZone(1, 0), makePestZone(3, 1)};
        l.goals = {
            {u"收获玉米 × 21"_s, GoalType::HarvestCount, "corn", 21, 0, false, StarTier::Star1},
            {u"效率 ≤ 420 tick"_s, GoalType::TickLimit, "", 420, 0, false, StarTier::Star2},
            {u"效率 ≤ 294 tick"_s, GoalType::TickLimit, "", 294, 0, false, StarTier::Star3}
        };
        levels_[9] = l;
    }

    // =====================================================================
    // 阶段4：数据结构入门（关10-13）— 元组 / 列表 / 累加 / 字典
    // =====================================================================

    // === 关卡 10：解包 (4×4) ===
    // 教学：元组解包、get_tick 计时
    {
        LevelConfig l;
        l.levelId = 10;
        l.name = u"解包"_s;
        l.description = u"用元组解包 a, b = ... 一次性赋值。get_tick() 返回当前 tick。目标：收获玉米×13"_s;
        l.gridW = 4; l.gridH = 4;
        l.maxTimeSec = 240;

        l.bugProbability = 0.005f;
        l.allowedFunctions = {"move","till","plant","water","fertilize","spray","wait","harvest","get_pos","get_map_size","get_current","get_tick","debug"};
        l.allowedSyntax = {"assign","if","else","for","while","def","return","and","or","not","break","continue","tuple"};
        l.allowedCrops = {"wheat","carrot","tomato","corn"};
        l.allowedBuiltins = {"range","tuple"};
        l.star2TickThreshold = 450;
        l.star3TickThreshold = 400;
        l.star3RequiredFeatures = {"tuple","get_tick"};
        l.tutorialCode = u"# 元组解包：一次给多个变量赋值\nx, y = get_pos()           # 替代 get_pos()[0]、get_pos()[1]\nw, h = get_map_size()\nprint(\"起点\", x, y, \"地图\", w, \"x\", h)\nstart = get_tick()\n\ndef go_home():\n    px, py = get_pos()     # 函数里也用解包\n    while px > 0:\n        move(\"left\")\n        px, py = get_pos()\n\ndef go_top():\n    px, py = get_pos()\n    while py > 0:\n        move(\"up\")\n        px, py = get_pos()\n\nfor row in range(4):\n    for col in range(4):\n        if get_current()[\"state\"] == \"empty\":\n            till()\n            plant(\"corn\")\n            water()\n            water()\n            fertilize()\n        if col < 3:\n            move(\"right\")\n    go_home()\n    if row < 3:\n        move(\"down\")\n\ngo_home()\ngo_top()\n\nfor t in range(18):\n    wait()\n    if t % 3 == 0:\n        for row in range(4):\n            for col in range(4):\n                cell = get_current()\n                if cell[\"hasBug\"]:\n                    spray()\n                elif cell[\"water\"] < 0.2:\n                    water()\n                if col < 3:\n                    move(\"right\")\n            go_home()\n            if row < 3:\n                move(\"down\")\n        go_home()\n        go_top()\n\ngo_home()\ngo_top()\nfor row in range(4):\n    for col in range(4):\n        if get_current()[\"state\"] == \"mature\":\n            harvest()\n        if col < 3:\n            move(\"right\")\n    go_home()\n    if row < 3:\n        move(\"down\")\n\nend = get_tick()\nprint(\"耗时\", end - start, \"tick\")\n"_s;
        l.presetCells = {makeRock(3, 0), makeRock(3, 2)};
        l.goals = {
            {u"收获玉米 × 13"_s, GoalType::HarvestCount, "corn", 13, 0, false, StarTier::Star1},
            {u"效率 ≤ 550 tick"_s, GoalType::TickLimit, "", 550, 0, false, StarTier::Star2},
            {u"使用新特性: 元组解包, get_tick"_s, GoalType::Custom, "", 1, 0, false, StarTier::Star3}
        };
        levels_[10] = l;
    }

    // === 关卡 11：列表 (5×5) ===
    // 教学：列表字面量、切片、len、append、enumerate
    {
        LevelConfig l;
        l.levelId = 11;
        l.name = u"列表"_s;
        l.description = u"用列表存储作物计划，用切片取子集，用 len 求长度。目标：收获小麦×4、胡萝卜×4、番茄×5、玉米×5"_s;
        l.gridW = 5; l.gridH = 5;
        l.maxTimeSec = 300;

        l.bugProbability = 0.006f;
        l.allowedFunctions = {"move","till","plant","water","fertilize","spray","wait","harvest","get_pos","get_map_size","get_current","debug"};
        l.allowedSyntax = {"assign","if","else","for","while","def","return","and","or","not","break","continue","tuple","list"};
        l.allowedCrops = {"wheat","carrot","tomato","corn"};
        l.allowedBuiltins = {"range","tuple","len","enumerate"};
        l.star2TickThreshold = 600;
        l.star3TickThreshold = 500;
        l.star3RequiredFeatures = {"list","len"};
        l.tutorialCode = u"# 列表字面量 [a, b, c]；切片 [1:3]；len；append\ncrops = [\"tomato\", \"carrot\", \"wheat\", \"corn\", \"wheat\"]\nprint(\"共\", len(crops), \"项\")\n\ndef plant_row(crop):\n    for col in range(5):\n        if get_current()[\"state\"] == \"empty\":\n            till()\n            plant(crop)\n            water()\n            if crop in [\"carrot\", \"tomato\"]:\n                water()\n            fertilize()\n        if col < 4:\n            move(\"right\")\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\ndef go_top():\n    while get_pos()[1] > 0:\n        move(\"up\")\n\n# 切片 crops[:5] 取全部 5 项，enumerate 带下标种 5 行\nfor i, c in enumerate(crops[:5]):\n    plant_row(c)\n    go_home()\n    if i < 4:\n        move(\"down\")\n\ngo_home()\ngo_top()\n\n# 等待成熟，巡逻除虫补水并收割\nfor i in range(16):\n    wait()\n    if i % 3 == 0:\n        for row in range(5):\n            for col in range(5):\n                cell = get_current()\n                if cell[\"hasBug\"]:\n                    spray()\n                elif cell[\"state\"] == \"mature\":\n                    harvest()\n                elif cell[\"water\"] < 0.15:\n                    water()\n                if col < 4:\n                    move(\"right\")\n            go_home()\n            if row < 4:\n                move(\"down\")\n        go_home()\n        go_top()\n\ngo_home()\ngo_top()\nfor row in range(5):\n    for col in range(5):\n        if get_current()[\"state\"] == \"mature\":\n            harvest()\n        if col < 4:\n            move(\"right\")\n    go_home()\n    if row < 4:\n        move(\"down\")\n"_s;
        l.presetCells = {makeRock(4, 2), makeRock(4, 4)};
        l.goals = {
            {u"收获小麦 × 4"_s, GoalType::HarvestCount, "wheat", 4, 0, false, StarTier::Star1},
            {u"收获胡萝卜 × 4"_s, GoalType::HarvestCount, "carrot", 4, 0, false, StarTier::Star1},
            {u"收获番茄 × 5"_s, GoalType::HarvestCount, "tomato", 5, 0, false, StarTier::Star1},
            {u"收获玉米 × 5"_s, GoalType::HarvestCount, "corn", 5, 0, false, StarTier::Star1},
            {u"效率 ≤ 750 tick"_s, GoalType::TickLimit, "", 750, 0, false, StarTier::Star2},
            {u"使用新特性: 列表, len"_s, GoalType::Custom, "", 1, 0, false, StarTier::Star3}
        };
        levels_[11] = l;
    }

    // === 关卡 12：累加 (5×5) ===
    // 教学：增强赋值 +=、min/max/sum、向日葵
    {
        LevelConfig l;
        l.levelId = 12;
        l.name = u"累加"_s;
        l.description = u"用 += 累加计数，用 min/max/sum 统计。向日葵不可收但加速邻居（过涝会死）。目标：收获番茄×15"_s;
        l.gridW = 5; l.gridH = 5;
        l.maxTimeSec = 300;

        l.bugProbability = 0.008f;
        l.allowedFunctions = {"move","till","plant","water","fertilize","spray","wait","harvest","get_pos","get_map_size","get_current","debug"};
        l.allowedSyntax = {"assign","if","else","for","while","def","return","and","or","not","break","continue","tuple","list","augassign"};
        l.allowedCrops = {"wheat","carrot","tomato","corn","sunflower"};
        l.allowedBuiltins = {"range","tuple","len","enumerate","min","max","sum"};
        l.star2TickThreshold = 250;
        l.star3TickThreshold = 210;
        l.star3RequiredFeatures = {"augassign","sum"};
        l.tutorialCode = u"# 增强赋值 += -=；min/max/sum；向日葵加速邻居\nharvested = 0\ncounts = []\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\ndef go_top():\n    while get_pos()[1] > 0:\n        move(\"up\")\n\n# 每3格种1向日葵，其余番茄（向日葵加速邻居）\nfor row in range(5):\n    for col in range(5):\n        if get_current()[\"state\"] == \"empty\":\n            till()\n            if (row + col) % 3 == 0:\n                plant(\"sunflower\")\n                water()\n            else:\n                plant(\"tomato\")\n                water()\n                fertilize()\n        if col < 4:\n            move(\"right\")\n    go_home()\n    if row < 4:\n        move(\"down\")\n\n# 等待成熟并巡逻除虫\nfor i in range(12):\n    wait()\n    if i % 3 == 0:\n        for row in range(5):\n            for col in range(5):\n                cell = get_current()\n                if cell[\"hasBug\"]:\n                    spray()\n                if col < 4:\n                    move(\"right\")\n            go_home()\n            if row < 4:\n                move(\"down\")\n        go_home()\n        go_top()\n\n# 收割并累加\ngo_home()\ngo_top()\nfor row in range(5):\n    row_count = 0\n    for col in range(5):\n        c = get_current()\n        if c[\"state\"] == \"mature\" and c[\"crop\"] != \"sunflower\":\n            harvest()\n            harvested += 1        # 等价 harvested = harvested + 1\n            row_count += 1\n        if col < 4:\n            move(\"right\")\n    counts.append(row_count)\n    go_home()\n    if row < 4:\n        move(\"down\")\n\nprint(\"共\", harvested, \"最少\", min(counts), \"最多\", max(counts), \"总和\", sum(counts))\n"_s;
        l.presetCells = {makeRock(4, 0), makeRock(4, 1), makePestZone(2, 1)};
        l.goals = {
            {u"收获番茄 × 15"_s, GoalType::HarvestCount, "tomato", 15, 0, false, StarTier::Star1},
            {u"效率 ≤ 350 tick"_s, GoalType::TickLimit, "", 350, 0, false, StarTier::Star2},
            {u"使用新特性: 增强赋值 +=, sum"_s, GoalType::Custom, "", 1, 0, false, StarTier::Star3}
        };
        levels_[12] = l;
    }

    // === 关卡 13：字典 (5×5) ===
    // 教学：字典字面量、items/keys/values
    {
        LevelConfig l;
        l.levelId = 13;
        l.name = u"字典"_s;
        l.description = u"用字典 {键: 值} 存储作物属性，用 items/keys/values 遍历。目标：收获小麦×4、胡萝卜×4、番茄×4、玉米×4"_s;
        l.gridW = 5; l.gridH = 5;
        l.maxTimeSec = 300;

        l.bugProbability = 0.008f;
        l.allowedFunctions = {"move","till","plant","water","fertilize","spray","wait","harvest","get_pos","get_map_size","get_current","debug"};
        l.allowedSyntax = {"assign","if","else","for","while","def","return","and","or","not","break","continue","tuple","list","augassign","dict"};
        l.allowedCrops = {"wheat","carrot","tomato","corn"};
        l.allowedBuiltins = {"range","tuple","len","enumerate","min","max","sum","dict","list"};
        l.star2TickThreshold = 220;
        l.star3TickThreshold = 190;
        l.star3RequiredFeatures = {"dict"};
        l.tutorialCode = u"# 字典 {键: 值}；用 [键] 取值；items/keys/values\ngrowth = {\"wheat\": 6, \"carrot\": 14, \"tomato\": 12, \"corn\": 20}\nwater_extra = {\"carrot\": 1, \"tomato\": 1}\ncrops = list(growth.keys())     # 取所有键转成列表\n\ndef plant_cell(crop):\n    till()\n    plant(crop)\n    water()\n    if crop in water_extra:\n        for _ in range(water_extra[crop]):\n            water()\n    fertilize()\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\ndef go_top():\n    while get_pos()[1] > 0:\n        move(\"up\")\n\n# 每行种一种，按字典决定等待\nfor i, c in enumerate(crops):\n    if i >= 4:\n        break\n    for col in range(5):\n        if get_current()[\"state\"] == \"empty\":\n            plant_cell(c)\n        if col < 4:\n            move(\"right\")\n    go_home()\n    if i < 3:\n        move(\"down\")\n\n# 按最长的玉米等待并巡逻除虫\nfor i in range(max(growth.values())):\n    wait()\n    if i % 3 == 0:\n        for row in range(4):\n            for col in range(5):\n                cell = get_current()\n                if cell[\"hasBug\"]:\n                    spray()\n                if col < 4:\n                    move(\"right\")\n            go_home()\n            if row < 3:\n                move(\"down\")\n        go_home()\n        go_top()\n\n# 遍历 items\nfor name, ticks in growth.items():\n    print(name, \"需\", ticks, \"tick\")\n\ngo_home()\ngo_top()\nfor row in range(4):\n    for col in range(5):\n        if get_current()[\"state\"] == \"mature\":\n            harvest()\n        if col < 4:\n            move(\"right\")\n    go_home()\n    if row < 3:\n        move(\"down\")\n"_s;
        l.presetCells = {makeRock(4, 3)};
        l.goals = {
            {u"收获小麦 × 4"_s, GoalType::HarvestCount, "wheat", 4, 0, false, StarTier::Star1},
            {u"收获胡萝卜 × 4"_s, GoalType::HarvestCount, "carrot", 4, 0, false, StarTier::Star1},
            {u"收获番茄 × 4"_s, GoalType::HarvestCount, "tomato", 4, 0, false, StarTier::Star1},
            {u"收获玉米 × 4"_s, GoalType::HarvestCount, "corn", 4, 0, false, StarTier::Star1},
            {u"效率 ≤ 300 tick"_s, GoalType::TickLimit, "", 300, 0, false, StarTier::Star2},
            {u"使用新特性: 字典"_s, GoalType::Custom, "", 1, 0, false, StarTier::Star3}
        };
        levels_[13] = l;
    }

    // =====================================================================
    // 阶段5：推导式与并行遍历（关14-16）— 列表推导式 / zip / all/any
    // =====================================================================

    // === 关卡 14：推导 (6×6) ===
    // 教学：列表推导式、get_goals
    {
        LevelConfig l;
        l.levelId = 14;
        l.name = u"推导"_s;
        l.description = u"用列表推导式 [x for x in xs if cond] 一行筛选。get_goals() 查询目标。目标：收获小麦×5、胡萝卜×5、番茄×6、玉米×6"_s;
        l.gridW = 6; l.gridH = 6;
        l.maxTimeSec = 300;

        l.bugProbability = 0.010f;
        l.allowedFunctions = {"move","till","plant","water","fertilize","spray","wait","harvest","get_pos","get_map_size","get_current","get_tick","get_goals","debug"};
        l.allowedSyntax = {"assign","if","else","for","while","def","return","and","or","not","break","continue","tuple","list","augassign","dict","listcomp"};
        l.allowedCrops = {"wheat","carrot","tomato","corn"};
        l.allowedBuiltins = {"range","tuple","len","enumerate","min","max","sum","dict","list"};
        l.star2TickThreshold = 800;
        l.star3TickThreshold = 700;
        l.star3RequiredFeatures = {"listcomp","get_goals"};
        l.tutorialCode = u"# 列表推导式 [表达式 for x in 列表 if 条件]\ncrops = [\"wheat\", \"carrot\", \"tomato\", \"corn\", \"tomato\", \"corn\"]\nharvestable = [c for c in crops if c != \"sunflower\"]    # 一行筛选\n\n# get_goals() 返回目标列表（字典）\ndef report():\n    goals = get_goals()\n    pending = [g for g in goals if not g[\"completed\"]]\n    print(\"剩余\", len(pending), \"个目标\")\n    return [g[\"description\"] for g in pending]\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\ndef go_top():\n    while get_pos()[1] > 0:\n        move(\"up\")\n\ndef plant_row(crop, count):\n    for i in range(count):\n        if get_current()[\"state\"] == \"empty\":\n            till()\n            plant(crop)\n            water()\n            if crop in [\"carrot\", \"tomato\"]:\n                water()\n            fertilize()\n        if i < count - 1:\n            move(\"right\")\n\n# 种 6 行（enumerate 带下标）\nfor i, c in enumerate(harvestable):\n    plant_row(c, 6)\n    go_home()\n    if i < 5:\n        move(\"down\")\n\ngo_home()\ngo_top()\nreport()\n\n# 等待成熟，巡逻除虫补水并收割\nfor t in range(20):\n    wait()\n    if t % 3 == 0:\n        for row in range(6):\n            for col in range(6):\n                cell = get_current()\n                if cell[\"hasBug\"]:\n                    spray()\n                elif cell[\"state\"] == \"mature\":\n                    harvest()\n                elif cell[\"water\"] < 0.15:\n                    water()\n                if col < 5:\n                    move(\"right\")\n            go_home()\n            if row < 5:\n                move(\"down\")\n        go_home()\n        go_top()\n\ngo_home()\ngo_top()\nfor row in range(6):\n    for col in range(6):\n        if get_current()[\"state\"] == \"mature\":\n            harvest()\n        if col < 5:\n            move(\"right\")\n    go_home()\n    if row < 5:\n        move(\"down\")\n\nreport()\n"_s;
        l.presetCells = {makeRock(5, 0), makeRock(5, 2), makePestZone(2, 1)};
        l.goals = {
            {u"收获小麦 × 5"_s, GoalType::HarvestCount, "wheat", 5, 0, false, StarTier::Star1},
            {u"收获胡萝卜 × 5"_s, GoalType::HarvestCount, "carrot", 5, 0, false, StarTier::Star1},
            {u"收获番茄 × 6"_s, GoalType::HarvestCount, "tomato", 6, 0, false, StarTier::Star1},
            {u"收获玉米 × 6"_s, GoalType::HarvestCount, "corn", 6, 0, false, StarTier::Star1},
            {u"效率 ≤ 1100 tick"_s, GoalType::TickLimit, "", 1100, 0, false, StarTier::Star2},
            {u"使用新特性: 列表推导式, get_goals"_s, GoalType::Custom, "", 1, 0, false, StarTier::Star3}
        };
        levels_[14] = l;
    }

    // === 关卡 15：并行 (6×6) ===
    // 教学：enumerate、zip、sorted
    {
        LevelConfig l;
        l.levelId = 15;
        l.name = u"并行"_s;
        l.description = u"用 enumerate 带下标，zip 并行遍历，sorted 排序。目标：收获小麦×5、胡萝卜×5、番茄×6、玉米×6"_s;
        l.gridW = 6; l.gridH = 6;
        l.maxTimeSec = 300;

        l.bugProbability = 0.010f;
        l.allowedFunctions = {"move","till","plant","water","fertilize","spray","wait","harvest","get_pos","get_map_size","get_current","get_tick","get_goals","debug"};
        l.allowedSyntax = {"assign","if","else","for","while","def","return","and","or","not","break","continue","tuple","list","augassign","dict","listcomp"};
        l.allowedCrops = {"wheat","carrot","tomato","corn","sunflower"};
        l.allowedBuiltins = {"range","tuple","len","enumerate","min","max","sum","dict","list","sorted","zip"};
        l.star2TickThreshold = 800;
        l.star3TickThreshold = 700;
        l.star3RequiredFeatures = {"sorted","zip"};
        l.tutorialCode = u"# enumerate 带下标；zip 并行遍历；sorted 排序\nplan = [(\"wheat\", 6), (\"carrot\", 6), (\"tomato\", 6), (\"corn\", 6), (\"tomato\", 6), (\"corn\", 6)]\ncrops = [c for c, _ in plan]            # 解包+推导复习\ncounts = [n for _, n in plan]\n\ndef plant_row(crop, count):\n    for i in range(count):\n        if get_current()[\"state\"] == \"empty\":\n            till()\n            plant(crop)\n            water()\n            if crop in [\"carrot\", \"tomato\"]:\n                water()\n            fertilize()\n        if i < count - 1:\n            move(\"right\")\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\ndef go_top():\n    while get_pos()[1] > 0:\n        move(\"up\")\n\n# enumerate：i 是行号，种 6 行\nfor i, c in enumerate(crops):\n    plant_row(c, counts[i])\n    go_home()\n    if i < 5:\n        move(\"down\")\n\n# zip：两两配对\nfor c, n in zip(crops, counts):\n    print(c, \"目标\", n)\n\n# sorted：按字母序排\nfor c in sorted(crops):\n    print(\"将种植\", c)\n\ngo_home()\ngo_top()\n\n# 等待成熟，巡逻除虫补水并收割\nfor t in range(20):\n    wait()\n    if t % 3 == 0:\n        for row in range(6):\n            for col in range(6):\n                cell = get_current()\n                if cell[\"hasBug\"]:\n                    spray()\n                elif cell[\"state\"] == \"mature\":\n                    harvest()\n                elif cell[\"water\"] < 0.15:\n                    water()\n                if col < 5:\n                    move(\"right\")\n            go_home()\n            if row < 5:\n                move(\"down\")\n        go_home()\n        go_top()\n\ngo_home()\ngo_top()\nfor row in range(6):\n    for col in range(6):\n        if get_current()[\"state\"] == \"mature\":\n            harvest()\n        if col < 5:\n            move(\"right\")\n    go_home()\n    if row < 5:\n        move(\"down\")\n"_s;
        l.presetCells = {makeRock(5, 0), makeRock(5, 3), makePestZone(2, 2)};
        l.goals = {
            {u"收获小麦 × 5"_s, GoalType::HarvestCount, "wheat", 5, 0, false, StarTier::Star1},
            {u"收获胡萝卜 × 5"_s, GoalType::HarvestCount, "carrot", 5, 0, false, StarTier::Star1},
            {u"收获番茄 × 6"_s, GoalType::HarvestCount, "tomato", 6, 0, false, StarTier::Star1},
            {u"收获玉米 × 6"_s, GoalType::HarvestCount, "corn", 6, 0, false, StarTier::Star1},
            {u"效率 ≤ 1100 tick"_s, GoalType::TickLimit, "", 1100, 0, false, StarTier::Star2},
            {u"使用新特性: sorted, zip"_s, GoalType::Custom, "", 1, 0, false, StarTier::Star3}
        };
        levels_[15] = l;
    }

    // === 关卡 16：判断 (6×6) ===
    // 教学：all/any、round/abs、嵌套函数
    {
        LevelConfig l;
        l.levelId = 16;
        l.name = u"判断"_s;
        l.description = u"用 all 全真才真、any 一真即真。round 取小数位、abs 绝对值。函数里可定义函数。目标：收获小麦×5、玉米×10"_s;
        l.gridW = 6; l.gridH = 6;
        l.maxTimeSec = 300;

        l.bugProbability = 0.012f;
        l.allowedFunctions = {"move","till","plant","water","fertilize","spray","wait","harvest","get_pos","get_map_size","get_current","get_tick","get_goals","debug"};
        l.allowedSyntax = {"assign","if","else","for","while","def","return","and","or","not","break","continue","tuple","list","augassign","dict","listcomp"};
        l.allowedCrops = {"wheat","carrot","tomato","corn","sunflower"};
        l.allowedBuiltins = {"range","tuple","len","enumerate","min","max","sum","dict","list","sorted","zip","all","any","round","abs"};
        l.star2TickThreshold = 700;
        l.star3TickThreshold = 600;
        l.star3RequiredFeatures = {"all","any"};
        l.tutorialCode = u"# all 全真才真；any 一真即真；round/abs；嵌套函数\n\ndef make_checker(target):\n    def check(cell):           # 内层函数，捕获外层 target\n        return cell[\"state\"] == target\n    return check               # 返回函数本身\n\nis_mature = make_checker(\"mature\")\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\ndef go_top():\n    while get_pos()[1] > 0:\n        move(\"up\")\n\n# 用 6 行：前 2 行小麦，后 4 行玉米，向日葵棋盘格混种\nfor row in range(6):\n    for col in range(6):\n        if get_current()[\"state\"] == \"empty\":\n            till()\n            if (row + col) % 3 == 0:\n                plant(\"sunflower\")\n                water()\n            elif row < 2:\n                plant(\"wheat\")\n                water()\n                fertilize()\n            else:\n                plant(\"corn\")\n                water()\n                fertilize()\n        if col < 5:\n            move(\"right\")\n    go_home()\n    if row < 5:\n        move(\"down\")\n\ngo_home()\ngo_top()\n\n# round / abs 示例\nprint(\"圆角\", round(22 / 7, 3), \"差值\", abs(3 - 10))\n\n# 等待成熟，巡逻除虫补水并收割\nfor t in range(16):\n    wait()\n    if t % 3 == 0:\n        for row in range(6):\n            for col in range(6):\n                cell = get_current()\n                if cell[\"hasBug\"]:\n                    spray()\n                elif cell[\"state\"] == \"mature\":\n                    harvest()\n                elif cell[\"water\"] < 0.15 and cell[\"crop\"] != \"sunflower\":\n                    water()\n                if col < 5:\n                    move(\"right\")\n            go_home()\n            if row < 5:\n                move(\"down\")\n        go_home()\n        go_top()\n\n# 用 any 判断是否有虫\ngo_home()\ngo_top()\nfor row in range(6):\n    row_cells = []\n    for col in range(6):\n        row_cells.append(get_current())\n        if col < 5:\n            move(\"right\")\n    if any([c[\"hasBug\"] for c in row_cells]):\n        print(\"第\", row, \"行有虫\")\n    go_home()\n    if row < 5:\n        move(\"down\")\n\ngo_home()\ngo_top()\nfor row in range(6):\n    for col in range(6):\n        if get_current()[\"state\"] == \"mature\":\n            harvest()\n        if col < 5:\n            move(\"right\")\n    go_home()\n    if row < 5:\n        move(\"down\")\n"_s;
        l.presetCells = {makeRock(5, 1), makeRock(5, 4), makePestZone(3, 0), makePestZone(0, 3)};
        l.goals = {
            {u"收获小麦 × 5"_s, GoalType::HarvestCount, "wheat", 5, 0, false, StarTier::Star1},
            {u"收获玉米 × 10"_s, GoalType::HarvestCount, "corn", 10, 0, false, StarTier::Star1},
            {u"效率 ≤ 1000 tick"_s, GoalType::TickLimit, "", 1000, 0, false, StarTier::Star2},
            {u"使用新特性: all/any"_s, GoalType::Custom, "", 1, 0, false, StarTier::Star3}
        };
        levels_[16] = l;
    }

    // =====================================================================
    // 阶段6：函数式进阶（关17-18）— lambda/闭包 / 字典推导式
    // =====================================================================

    // === 关卡 17：闭包 (6×6) ===
    // 教学：lambda、默认参数、闭包
    {
        LevelConfig l;
        l.levelId = 17;
        l.name = u"闭包"_s;
        l.description = u"用 lambda 写匿名小函数，用默认参数，用闭包记住变量。目标：收获小麦×5、玉米×10"_s;
        l.gridW = 6; l.gridH = 6;
        l.maxTimeSec = 300;

        l.bugProbability = 0.012f;
        l.allowedFunctions = {"move","till","plant","water","fertilize","spray","wait","harvest","get_pos","get_map_size","get_current","get_tick","get_goals","debug"};
        l.allowedSyntax = {"assign","if","else","for","while","def","return","and","or","not","break","continue","tuple","list","augassign","dict","listcomp","lambda"};
        l.allowedCrops = {"wheat","carrot","tomato","corn","sunflower"};
        l.allowedBuiltins = {"range","tuple","len","enumerate","min","max","sum","dict","list","sorted","zip","all","any","round","abs"};
        l.star2TickThreshold = 600;
        l.star3TickThreshold = 500;
        l.star3RequiredFeatures = {"lambda"};
        l.tutorialCode = u"# lambda 匿名小函数；默认参数；闭包\nsquare = lambda x: x * x\nprint(square(6))           # 36\n\ndef patrol(times=3, step=2):\n    for t in range(times):\n        wait()\n        if t % step == 0:\n            for row in range(6):\n                for col in range(6):\n                    cell = get_current()\n                    if cell[\"hasBug\"]:\n                        spray()\n                    elif cell[\"state\"] == \"mature\":\n                        harvest()\n                    elif cell[\"water\"] < 0.15 and cell[\"crop\"] != \"sunflower\":\n                        water()\n                    if col < 5:\n                        move(\"right\")\n                go_home()\n                if row < 5:\n                    move(\"down\")\n            go_home()\n            go_top()\n\n# 闭包：函数记住创建时的变量\ndef make_multiplier(factor):\n    return lambda x: x * factor    # 捕获 factor\n\ndouble = make_multiplier(2)\ntriple = make_multiplier(3)\nprint(double(5), triple(5))   # 10 15\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\ndef go_top():\n    while get_pos()[1] > 0:\n        move(\"up\")\n\n# 用 6 行：前 2 行小麦，后 4 行玉米，向日葵棋盘格混种\nfor row in range(6):\n    for col in range(6):\n        if get_current()[\"state\"] == \"empty\":\n            till()\n            if (row + col) % 3 == 0:\n                plant(\"sunflower\")\n                water()\n            elif row < 2:\n                plant(\"wheat\")\n                water()\n                fertilize()\n            else:\n                plant(\"corn\")\n                water()\n                fertilize()\n        if col < 5:\n            move(\"right\")\n    go_home()\n    if row < 5:\n        move(\"down\")\n\ngo_home()\ngo_top()\n\n# sorted 用 lambda 作 key\ngoals = get_goals()\nhardest = sorted(goals, key=lambda g: g[\"target\"], reverse=True)\nprint(\"最难:\", hardest[0][\"description\"])\n\npatrol()          # 用默认值\npatrol(5, 3)      # 覆盖默认\n\ngo_home()\ngo_top()\nfor row in range(6):\n    for col in range(6):\n        if get_current()[\"state\"] == \"mature\":\n            harvest()\n        if col < 5:\n            move(\"right\")\n    go_home()\n    if row < 5:\n        move(\"down\")\n"_s;
        l.presetCells = {makeRock(5, 2), makeRock(5, 5), makePestZone(2, 3)};
        l.goals = {
            {u"收获小麦 × 5"_s, GoalType::HarvestCount, "wheat", 5, 0, false, StarTier::Star1},
            {u"收获玉米 × 10"_s, GoalType::HarvestCount, "corn", 10, 0, false, StarTier::Star1},
            {u"效率 ≤ 900 tick"_s, GoalType::TickLimit, "", 900, 0, false, StarTier::Star2},
            {u"使用新特性: lambda"_s, GoalType::Custom, "", 1, 0, false, StarTier::Star3}
        };
        levels_[17] = l;
    }

    // === 关卡 18：大农场 (7×7) ===
    // 教学：字典推导式、综合内置函数
    {
        LevelConfig l;
        l.levelId = 18;
        l.name = u"大农场"_s;
        l.description = u"7×7 大农场，用字典推导式 {k: v for ...} 建表，综合运用所有技巧。目标：收获小麦×7、胡萝卜×7、番茄×7、玉米×7"_s;
        l.gridW = 7; l.gridH = 7;
        l.maxTimeSec = 360;

        l.bugProbability = 0.014f;
        l.allowedFunctions = {"move","till","plant","water","fertilize","spray","wait","harvest","get_pos","get_map_size","get_current","get_tick","get_goals","debug"};
        l.allowedSyntax = {"assign","if","else","for","while","def","return","and","or","not","break","continue","tuple","list","augassign","dict","listcomp","lambda","dictcomp"};
        l.allowedCrops = {"wheat","carrot","tomato","corn","sunflower"};
        l.allowedBuiltins = {"range","tuple","len","enumerate","min","max","sum","dict","list","sorted","zip","all","any","round","abs"};
        l.star2TickThreshold = 1000;
        l.star3TickThreshold = 900;
        l.star3RequiredFeatures = {"dictcomp"};
        l.tutorialCode = u"# 字典推导式 {k: v for ...}\ngrowth = {c: 6 + i * 3 for i, c in enumerate([\"wheat\", \"carrot\", \"tomato\", \"corn\"])}\ntotals = {c: 0 for c in growth}       # 初始化计数器\nneeded = {c: 7 for c in growth}        # 每种需要 7 个\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\ndef go_top():\n    while get_pos()[1] > 0:\n        move(\"up\")\n\ndef plant_cell(crop):\n    till()\n    plant(crop)\n    water()\n    if crop == \"carrot\":\n        water()\n    if crop != \"sunflower\":\n        fertilize()\n\n# 遍历所有格子，用字典 needed 记录还需种几个\nfor row in range(7):\n    for col in range(7):\n        if get_current()[\"state\"] == \"empty\":\n            for c, n in needed.items():\n                if n > 0:\n                    plant_cell(c)\n                    needed[c] -= 1\n                    break\n        if col < 6:\n            move(\"right\")\n    go_home()\n    if row < 6:\n        move(\"down\")\n\ngo_home()\ngo_top()\n\n# 等待成熟，巡逻除虫补水并收割\nfor t in range(16):\n    wait()\n    if t % 4 == 0:\n        for row in range(7):\n            for col in range(7):\n                cell = get_current()\n                if cell[\"hasBug\"]:\n                    spray()\n                elif cell[\"state\"] == \"mature\" and cell[\"crop\"] != \"sunflower\":\n                    harvest()\n                    crop = cell[\"crop\"]\n                    if crop in totals:\n                        totals[crop] += 1\n                elif cell[\"water\"] < 0.1 and cell[\"crop\"] != \"sunflower\":\n                    water()\n                if col < 6:\n                    move(\"right\")\n            go_home()\n            if row < 6:\n                move(\"down\")\n        go_home()\n        go_top()\n\ngo_home()\ngo_top()\nfor row in range(7):\n    for col in range(7):\n        c = get_current()\n        if c[\"state\"] == \"mature\" and c[\"crop\"] != \"sunflower\":\n            harvest()\n            crop = c[\"crop\"]\n            if crop in totals:\n                totals[crop] += 1\n        if col < 6:\n            move(\"right\")\n    go_home()\n    if row < 6:\n        move(\"down\")\n\n# 综合用 sum / max\nprint(\"总收割\", sum(totals.values()), \"最多\", max(totals, key=totals.get))\n"_s;
        l.presetCells = {
            makeRock(6, 0), makeRock(6, 1), makeRock(6, 2), makeRock(6, 3),
            makeRock(6, 4), makeRock(6, 5),
            makeRock(1, 6), makeRock(4, 6),
            makePestZone(2, 1), makePestZone(5, 2), makePestZone(0, 5)
        };
        l.goals = {
            {u"收获小麦 × 7"_s, GoalType::HarvestCount, "wheat", 7, 0, false, StarTier::Star1},
            {u"收获胡萝卜 × 7"_s, GoalType::HarvestCount, "carrot", 7, 0, false, StarTier::Star1},
            {u"收获番茄 × 7"_s, GoalType::HarvestCount, "tomato", 7, 0, false, StarTier::Star1},
            {u"收获玉米 × 7"_s, GoalType::HarvestCount, "corn", 7, 0, false, StarTier::Star1},
            {u"效率 ≤ 1500 tick"_s, GoalType::TickLimit, "", 1500, 0, false, StarTier::Star2},
            {u"使用新特性: 字典推导式"_s, GoalType::Custom, "", 1, 0, false, StarTier::Star3}
        };
        levels_[18] = l;
    }

    // =====================================================================
    // 阶段7：生成器与综合（关19-20）
    // =====================================================================

    // === 关卡 19：生成器 (7×7) ===
    // 教学：生成器表达式、集合推导式
    {
        LevelConfig l;
        l.levelId = 19;
        l.name = u"生成器"_s;
        l.description = u"用生成器表达式 (x for x in xs) 惰性求值，用集合推导式 {x for x in xs} 去重。目标：收获玉米×21"_s;
        l.gridW = 7; l.gridH = 7;
        l.maxTimeSec = 300;

        l.bugProbability = 0.015f;
        l.allowedFunctions = {"move","till","plant","water","fertilize","spray","wait","harvest","get_pos","get_map_size","get_current","get_tick","get_goals","debug"};
        l.allowedSyntax = {"assign","if","else","for","while","def","return","and","or","not","break","continue","tuple","list","augassign","dict","listcomp","lambda","dictcomp","genexp","setcomp"};
        l.allowedCrops = {"wheat","carrot","tomato","corn","sunflower"};
        l.allowedBuiltins = {"range","tuple","len","enumerate","min","max","sum","dict","list","sorted","zip","all","any","round","abs"};
        l.star2TickThreshold = 650;
        l.star3TickThreshold = 550;
        l.star3RequiredFeatures = {"genexp","setcomp"};
        l.tutorialCode = u"# 生成器表达式 (expr for x in xs) —— 惰性，省内存\n# 集合推导式 {expr for x in xs} —— 去重\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\ndef go_top():\n    while get_pos()[1] > 0:\n        move(\"up\")\n\ndef scan_all():\n    cells = []\n    for row in range(7):\n        for col in range(7):\n            cells.append(get_current())\n            if col < 6:\n                move(\"right\")\n        go_home()\n        if row < 6:\n            move(\"down\")\n    go_home()\n    go_top()\n    return cells\n\n# 棋盘格：向日葵 + 玉米\nfor row in range(7):\n    for col in range(7):\n        if get_current()[\"state\"] == \"empty\":\n            till()\n            if (row + col) % 2 == 0:\n                plant(\"sunflower\")\n                water()\n            else:\n                plant(\"corn\")\n                water()\n                fertilize()\n        if col < 6:\n            move(\"right\")\n    go_home()\n    if row < 6:\n        move(\"down\")\n\ngo_home()\ngo_top()\n\ncells = scan_all()\n# 生成器表达式配合 sum\nbug_count = sum(1 for c in cells if c[\"hasBug\"])\nif any(c[\"hasBug\"] for c in cells):\n    print(\"发现\", bug_count, \"个虫，开始巡逻\")\n# 集合推导式去重\nkinds = {c[\"crop\"] for c in cells if c[\"crop\"]}\nprint(\"种植种类:\", kinds)\n\nfor t in range(14):\n    wait()\n    if t % 3 == 0:\n        for row in range(7):\n            for col in range(7):\n                cell = get_current()\n                if cell[\"hasBug\"]:\n                    spray()\n                elif cell[\"water\"] < 0.1 and cell[\"crop\"] != \"sunflower\":\n                    water()\n                elif cell[\"state\"] == \"mature\":\n                    harvest()\n                if col < 6:\n                    move(\"right\")\n            go_home()\n            if row < 6:\n                move(\"down\")\n        go_home()\n        go_top()\n\ngo_home()\ngo_top()\nfor row in range(7):\n    for col in range(7):\n        if get_current()[\"state\"] == \"mature\":\n            harvest()\n        if col < 6:\n            move(\"right\")\n    go_home()\n    if row < 6:\n        move(\"down\")\n"_s;
        l.presetCells = {
            makeRock(6, 0), makeRock(6, 1), makeRock(6, 2), makeRock(6, 3),
            makeRock(6, 4), makeRock(6, 5), makeRock(6, 6),
            makePestZone(2, 0), makePestZone(5, 1), makePestZone(0, 4), makePestZone(4, 6)
        };
        l.goals = {
            {u"收获玉米 × 21"_s, GoalType::HarvestCount, "corn", 21, 0, false, StarTier::Star1},
            {u"效率 ≤ 950 tick"_s, GoalType::TickLimit, "", 950, 0, false, StarTier::Star2},
            {u"使用新特性: 生成器表达式, 集合推导式"_s, GoalType::Custom, "", 1, 0, false, StarTier::Star3}
        };
        levels_[19] = l;
    }

    // === 关卡 20：终极农场 (8×8) ===
    // 全 API + 全语法 + 全作物 + 全机制
    {
        LevelConfig l;
        l.levelId = 20;
        l.name = u"终极农场"_s;
        l.description = u"8×8 终极农场，综合运用所有技能。尝试用推导式、lambda、闭包写出最优雅的解法。目标：收获小麦×7、胡萝卜×7、番茄×7、玉米×7"_s;
        l.gridW = 8; l.gridH = 8;
        l.maxTimeSec = 420;

        l.bugProbability = 0.018f;
        l.allowedFunctions = {"move","till","plant","water","fertilize","spray","wait","harvest","get_pos","get_map_size","get_current","get_tick","get_goals","debug"};
        l.allowedSyntax = {"assign","if","else","for","while","def","return","and","or","not","break","continue","tuple","list","augassign","dict","listcomp","lambda","dictcomp","genexp","setcomp"};
        l.allowedCrops = {"wheat","carrot","tomato","corn","sunflower"};
        l.allowedBuiltins = {"range","tuple","len","enumerate","min","max","sum","dict","list","sorted","zip","all","any","round","abs"};
        l.star2TickThreshold = 1200;
        l.star3TickThreshold = 1000;
        l.star3RequiredFeatures = {"dictcomp","genexp","lambda"};
        l.tutorialCode = u"# 终极农场！全 API + 全语法 + 全机制\n\ndef go_home():\n    while get_pos()[0] > 0:\n        move(\"left\")\n\ndef go_top():\n    while get_pos()[1] > 0:\n        move(\"up\")\n\n# 棋盘格布局：向日葵 + 四种作物（按行分配）\nfor row in range(8):\n    for col in range(8):\n        if get_current()[\"state\"] == \"empty\":\n            till()\n            if row < 6 and (row + col) % 2 == 0:\n                plant(\"sunflower\")\n                water()\n            elif row < 2:\n                plant(\"wheat\")\n                water()\n                fertilize()\n            elif row < 4:\n                plant(\"carrot\")\n                water()\n                water()\n                fertilize()\n            elif row < 6:\n                plant(\"tomato\")\n                water()\n                fertilize()\n            elif row < 7:\n                plant(\"corn\")\n                water()\n                fertilize()\n        if col < 7:\n            move(\"right\")\n    go_home()\n    if row < 7:\n        move(\"down\")\n\ngo_home()\ngo_top()\n\n# 智能巡逻（最优化）—— 用 lambda 作 key 排目标\ndef patrol():\n    for row in range(8):\n        for col in range(8):\n            cell = get_current()\n            if cell[\"hasBug\"]:\n                spray()\n            elif cell[\"state\"] == \"mature\":\n                harvest()\n            elif cell[\"water\"] < 0.1 and cell[\"crop\"] != \"sunflower\":\n                water()\n            if col < 7:\n                move(\"right\")\n        go_home()\n        if row < 7:\n            move(\"down\")\n    go_home()\n    go_top()\n\n# 用生成器表达式统计\ngoals = get_goals()\nremaining = sum(1 for g in goals if not g[\"completed\"])\nprint(\"剩余目标\", remaining)\n\n# 持续巡逻\nfor t in range(25):\n    wait()\n    if t % 2 == 0:\n        patrol()\n\n# 最终收割\ngo_home()\ngo_top()\nfor row in range(8):\n    for col in range(8):\n        if get_current()[\"state\"] == \"mature\":\n            harvest()\n        if col < 7:\n            move(\"right\")\n    go_home()\n    if row < 7:\n        move(\"down\")\n"_s;
        l.presetCells = {
            makeRock(7, 0), makeRock(7, 1), makeRock(7, 2), makeRock(7, 3),
            makeRock(7, 4), makeRock(7, 5), makeRock(7, 6), makeRock(7, 7),
            makePestZone(3, 1), makePestZone(6, 2), makePestZone(0, 5),
            makePestZone(5, 6)
        };
        l.goals = {
            {u"收获小麦 × 7"_s, GoalType::HarvestCount, "wheat", 7, 0, false, StarTier::Star1},
            {u"收获胡萝卜 × 7"_s, GoalType::HarvestCount, "carrot", 7, 0, false, StarTier::Star1},
            {u"收获番茄 × 7"_s, GoalType::HarvestCount, "tomato", 7, 0, false, StarTier::Star1},
            {u"收获玉米 × 7"_s, GoalType::HarvestCount, "corn", 7, 0, false, StarTier::Star1},
            {u"效率 ≤ 1800 tick"_s, GoalType::TickLimit, "", 1800, 0, false, StarTier::Star2},
            {u"使用新特性: 字典推导式, 生成器表达式, lambda"_s, GoalType::Custom, "", 1, 0, false, StarTier::Star3}
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
    QVariantList allowedBuiltins;
    QStringList allowedBuiltinNames = l.allowedBuiltins.values();
    allowedBuiltinNames.sort();
    for (const auto &b : allowedBuiltinNames) {
        allowedBuiltins.append(b);
    }
    QStringList star3Features = l.star3RequiredFeatures.values();
    star3Features.sort();
    return {
        {"id", l.levelId}, {"name", l.name},
        {"description", l.description},
        {"tutorialCode", l.tutorialCode},
        {"gridW", l.gridW}, {"gridH", l.gridH},
        {"maxTimeSec", l.maxTimeSec},
        {"star2TickThreshold", l.star2TickThreshold},
        {"star3TickThreshold", l.star3TickThreshold},
        {"star3RequiredFeatures", star3Features},
        {"goals", goals},
        {"allowedFunctions", allowedFunctions},
        {"allowedBuiltins", allowedBuiltins}
    };
}

QVariantMap LevelManager::getLevelNewContent(int id) const {
    if (!levels_.contains(id)) return {};

    const auto &curr = levels_[id];

    // Get previous level's allowed sets
    QSet<QString> prevFunctions;
    QSet<QString> prevSyntax;
    QSet<QString> prevCrops;
    QSet<QString> prevBuiltins;
    if (id > 1 && levels_.contains(id - 1)) {
        const auto &prev = levels_[id - 1];
        prevFunctions = prev.allowedFunctions;
        prevSyntax = prev.allowedSyntax;
        prevCrops = prev.allowedCrops;
        prevBuiltins = prev.allowedBuiltins;
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
        "break","continue","return","and","or","not",
        "augassign","tuple","list","dict","listcomp",
        "dictcomp","setcomp","genexp","lambda"
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

    QVariantList newBuiltins;
    QStringList newBuiltinList;
    for (const auto &b : curr.allowedBuiltins) {
        if (!prevBuiltins.contains(b)) {
            newBuiltinList.append(b);
        }
    }
    newBuiltinList.sort();
    for (const auto &b : newBuiltinList) {
        newBuiltins.append(b);
    }

    return {
        {"newFunctions", newFunctions},
        {"newSyntax", newSyntax},
        {"newCrops", newCrops},
        {"newBuiltins", newBuiltins}
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
