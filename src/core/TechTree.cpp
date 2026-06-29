#include "TechTree.h"

TechTree::TechTree(QObject *parent)
    : QObject(parent)
{
    buildGraph();
    // Starting unlocks
    m_unlocked.insert("A0");
    m_unlocked.insert("B0");
    m_unlocked.insert("C0");
    m_unlocked.insert("D0");
}

void TechTree::buildGraph() {
    m_nodes = {
        // Branch A: Agriculture
        {"A0", TechBranch::A_Agriculture, "草莓试种", 0, {}, "解锁新手作物草莓"},
        {"A1", TechBranch::A_Agriculture, "番茄温室", 70, {"A0", "C2"}, "解锁可持续结果的番茄"},
        {"A2", TechBranch::A_Agriculture, "增产技术", 140, {"A1", "C3"}, "作物产量 +20%"},
        {"A3", TechBranch::A_Agriculture, "葡萄架", 180, {"A1", "D2"}, "解锁依赖肥力的葡萄"},
        {"A4", TechBranch::A_Agriculture, "抗病研究", 260, {"A2", "C4"}, "抗病能力 +30%"},
        {"A5", TechBranch::A_Agriculture, "西瓜种植", 360, {"A3", "D3"}, "解锁一次性高收益西瓜"},
        {"A6", TechBranch::A_Agriculture, "菠萝种植", 650, {"A5", "C7"}, "解锁高利润菠萝"},

        // Branch B: Mechanics
        {"B0", TechBranch::B_Mechanics, "基础机械", 0, {}, "解锁基础无人机"},
        {"B1", TechBranch::B_Mechanics, "无人机速度", 90, {"B0", "D2"}, "无人机速度 +20%"},
        {"B2", TechBranch::B_Mechanics, "第二架无人机", 220, {"B1", "C4"}, "无人机数量 +1"},
        {"B3", TechBranch::B_Mechanics, "智能寻路", 360, {"B1", "C6", "D3"}, "解锁 pathfind()"},
        {"B4", TechBranch::B_Mechanics, "多机协作", 720, {"B2", "C7"}, "无人机数量 +2"},

        // Branch C: Programming
        {"C0", TechBranch::C_Programming, "基础编程", 0, {}, "解锁 plant()/harvest()/wait()"},
        {"C1", TechBranch::C_Programming, "方向移动", 35, {"C0", "D1"}, "解锁 move_up/down/left/right"},
        {"C2", TechBranch::C_Programming, "基础灌溉", 60, {"C1", "B0"}, "解锁 water() 与 fertilize()"},
        {"C3", TechBranch::C_Programming, "扫描判断", 100, {"C2", "A1"}, "解锁 scan() 与 if/else"},
        {"C4", TechBranch::C_Programming, "循环耕作", 160, {"C3", "D2"}, "解锁 for/range"},
        {"C5", TechBranch::C_Programming, "持续巡田", 220, {"C4", "B1"}, "解锁 while/break/continue"},
        {"C6", TechBranch::C_Programming, "函数封装", 300, {"C5", "D3"}, "解锁 def/return"},
        {"C7", TechBranch::C_Programming, "数据结构", 420, {"C6", "A5"}, "解锁 list/dict"},
        {"C8", TechBranch::C_Programming, "调度系统", 850, {"C7", "B3", "B4"}, "解锁 schedule()"},

        // Branch D: Land
        {"D0", TechBranch::D_Land, "基础土地", 0, {}, "1×1 地图"},
        {"D1", TechBranch::D_Land, "扩建 2×2", 60, {"D0", "A0"}, "地图扩大至 2×2，让移动代码有意义"},
        {"D2", TechBranch::D_Land, "扩建 3×3", 260, {"D1", "C2"}, "地图扩大至 3×3，开始循环自动化"},
        {"D3", TechBranch::D_Land, "扩建 5×5", 650, {"D2", "C4"}, "地图扩大至 5×5，进入规模化耕作"},
        {"D4", TechBranch::D_Land, "土壤改良", 240, {"C2", "A1"}, "肥力上限 +50"},
        {"D5", TechBranch::D_Land, "开垦提速", 320, {"D2", "B1"}, "开垦速度增加"},
        {"D6", TechBranch::D_Land, "扩建 8×8", 2000, {"D3"}, "地图扩大至 8×8"},
        {"D7", TechBranch::D_Land, "扩建 15×15", 5000, {"D6"}, "地图扩大至 15×15"},
        {"D8", TechBranch::D_Land, "扩建 25×25", 15000, {"D7"}, "地图扩大至 25×25"},
        {"D9", TechBranch::D_Land, "扩建 50×50", 50000, {"D8"}, "地图扩大至 50×50"},
    };
}

bool TechTree::isUnlocked(const QString &id) const {
    return m_unlocked.contains(id);
}

bool TechTree::canUnlock(const QString &id) const {
    if (m_unlocked.contains(id)) return false;
    for (auto &node : m_nodes) {
        if (node.id == id) {
            for (auto &prereq : node.prerequisites)
                if (!m_unlocked.contains(prereq)) return false;
            return true;
        }
    }
    return false;
}

bool TechTree::unlock(const QString &id) {
    if (!canUnlock(id)) return false;
    m_unlocked.insert(id);
    emit nodeUnlocked(id);
    return true;
}

double TechTree::yieldMultiplier() const {
    double mult = 1.0;
    if (m_unlocked.contains("A2")) mult += 0.20;
    return mult;
}

double TechTree::diseaseResistBonus() const {
    double bonus = 0.0;
    if (m_unlocked.contains("A4")) bonus += 0.30;
    return bonus;
}

double TechTree::droneSpeedBonus() const {
    double bonus = 0.0;
    if (m_unlocked.contains("B1")) bonus += 0.20;
    return bonus;
}

int TechTree::extraDrones() const {
    int extra = 0;
    if (m_unlocked.contains("B2")) extra += 1;
    if (m_unlocked.contains("B4")) extra += 2;
    return extra;
}

bool TechTree::hasSmartPathfinding() const {
    return m_unlocked.contains("B3");
}

double TechTree::cultivationSpeedMultiplier() const {
    return m_unlocked.contains("D5") ? 2.0 : 1.0;
}

double TechTree::fertilityCapBonus() const {
    return m_unlocked.contains("D4") ? 50.0 : 0.0;
}
