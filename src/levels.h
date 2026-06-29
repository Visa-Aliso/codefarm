#pragma once

#include "types.h"

inline const QVector<CropInfo> kCrops = {
    {"wheat", "小麦", "麦", 12, 0.00, 0.006},
    {"carrot", "胡萝卜", "胡", 18, 0.35, 0.009},
    {"tomato", "番茄", "茄", 15, 0.20, 0.016},
    {"corn", "玉米", "玉", 28, 0.22, 0.010},
    {"sunflower", "向日葵", "葵", 40, 0.18, 0.013},
    {"berry", "浆果", "莓", 22, 0.45, 0.018},
};

inline const CropInfo *cropInfo(const QString &key)
{
    for (const auto &crop : kCrops) {
        if (crop.key == key) return &crop;
    }
    return nullptr;
}

inline QVector<Level> buildLevels()
{
    const QString allFuncs = "move,till,plant,water,fertilize,debug,harvest,get_pos,get_current,get_goals";
    auto funcs = [](const QString &csv) { return csv.split(',', Qt::SkipEmptyParts); };
    return {
        {1, "初次收割", "认识无人机，完成第一块小麦田", {1, 1}, 60, {{"wheat", 1, 0}}, funcs("till,plant,water,harvest"), {"顺序执行"}, "till()\nplant(\"wheat\")\nwater()\nharvest()", ""},
        {2, "移动与修复", "学习移动与 debug，清理小麦田", {2, 2}, 80, {{"wheat", 3, 0}}, funcs("move,till,plant,water,debug,harvest,get_pos"), {"if / else"}, "till()\nplant(\"wheat\")\nwater()\ndebug()\nharvest()\nmove(\"right\")", ""},
        {3, "浇水的烦恼", "胡萝卜需要维持水分", {2, 2}, 95, {{"carrot", 3, 0}}, funcs("move,till,plant,water,harvest,get_pos"), {"条件判断"}, "till()\nplant(\"carrot\")\nwater()\nharvest()\nmove(\"right\")", ""},
        {4, "九宫格巡航", "用固定路线覆盖 3x3 地图", {3, 3}, 120, {{"wheat", 6, 0}}, funcs("move,till,plant,water,harvest,get_pos,get_current"), {"for / range"}, "till()\nplant(\"wheat\")\nwater()\nharvest()\nmove(\"right\")", ""},
        {5, "番茄竞速", "预开垦土地，靠施肥提高效率", {3, 3}, 115, {{"tomato", 6, 0}}, funcs("move,plant,water,fertilize,debug,harvest,get_current"), {"while / break"}, "plant(\"tomato\")\nfertilize()\nwater()\ndebug()\nharvest()\nmove(\"right\")", "tilled"},
        {6, "玉米农场", "慢作物需要周期维护", {5, 5}, 170, {{"corn", 10, 0}}, funcs(allFuncs), {"def / for / while"}, "till()\nplant(\"corn\")\nfertilize()\nwater()\ndebug()\nharvest()\nmove(\"right\")", ""},
        {7, "混合种植", "同时完成小麦与胡萝卜目标", {5, 5}, 190, {{"wheat", 8, 0}, {"carrot", 6, 0}}, funcs(allFuncs), {"全部基础语法"}, "till()\nplant(\"wheat\")\nwater()\nharvest()\nmove(\"right\")", ""},
        {8, "向日葵的赌注", "高价值作物成长慢，维护窗口更长", {5, 5}, 240, {{"sunflower", 5, 0}, {"wheat", 8, 0}}, funcs(allFuncs), {"综合调度"}, "till()\nplant(\"sunflower\")\nfertilize()\nwater()\nharvest()\nmove(\"right\")", ""},
        {9, "湿地浆果", "高水分作物对浇水节奏更敏感", {4, 4}, 180, {{"berry", 7, 0}}, funcs(allFuncs), {"状态读取"}, "till()\nplant(\"berry\")\nwater()\nfertilize()\nharvest()\nmove(\"right\")", ""},
        {10, "双线订单", "番茄和玉米共享同一片田", {5, 4}, 210, {{"tomato", 8, 0}, {"corn", 5, 0}}, funcs(allFuncs), {"函数拆分"}, "till()\nplant(\"tomato\")\nwater()\nfertilize()\nharvest()\nmove(\"right\")", ""},
        {11, "干旱预警", "所有土地初始缺水，需要先补给", {5, 5}, 210, {{"carrot", 10, 0}}, funcs(allFuncs), {"循环维护"}, "till()\nplant(\"carrot\")\nwater()\nwater()\nharvest()\nmove(\"right\")", "dry"},
        {12, "虫害高峰", "Bug 概率提升，debug 更关键", {6, 5}, 240, {{"tomato", 10, 0}}, funcs(allFuncs), {"异常处理思路"}, "till()\nplant(\"tomato\")\nfertilize()\nwater()\ndebug()\nharvest()\nmove(\"right\")", "bugs"},
        {13, "长垄巡田", "宽地图考验移动路线", {7, 4}, 230, {{"wheat", 16, 0}}, funcs(allFuncs), {"蛇形遍历"}, "till()\nplant(\"wheat\")\nwater()\nharvest()\nmove(\"right\")", ""},
        {14, "三色农场", "三种作物并行完成", {6, 5}, 260, {{"wheat", 8, 0}, {"carrot", 8, 0}, {"tomato", 8, 0}}, funcs(allFuncs), {"策略分区"}, "till()\nplant(\"wheat\")\nwater()\nharvest()\nmove(\"right\")", ""},
        {15, "肥料预算", "预置开垦地，依靠 fertilize 冲刺", {6, 6}, 270, {{"corn", 12, 0}, {"tomato", 10, 0}}, funcs(allFuncs), {"资源优先级"}, "plant(\"corn\")\nfertilize()\nwater()\ndebug()\nharvest()\nmove(\"right\")", "tilled"},
        {16, "夜间收割", "时间更紧，目标更集中", {5, 5}, 165, {{"wheat", 14, 0}, {"berry", 5, 0}}, funcs(allFuncs), {"时间约束"}, "till()\nplant(\"berry\")\nwater()\nfertilize()\nharvest()\nmove(\"right\")", ""},
        {17, "温室矩阵", "大地图综合种植挑战", {7, 6}, 330, {{"carrot", 12, 0}, {"tomato", 12, 0}, {"berry", 8, 0}}, funcs(allFuncs), {"综合路线"}, "till()\nplant(\"carrot\")\nwater()\nfertilize()\ndebug()\nharvest()\nmove(\"right\")", ""},
        {18, "金色花海", "向日葵目标扩大，容错降低", {6, 6}, 360, {{"sunflower", 9, 0}, {"wheat", 12, 0}}, funcs(allFuncs), {"长周期维护"}, "till()\nplant(\"sunflower\")\nfertilize()\nwater()\ndebug()\nharvest()\nmove(\"right\")", ""},
        {19, "全品类订单", "所有作物都要交付", {7, 6}, 390, {{"wheat", 8, 0}, {"carrot", 8, 0}, {"tomato", 8, 0}, {"corn", 6, 0}, {"berry", 6, 0}}, funcs(allFuncs), {"生产排程"}, "till()\nplant(\"wheat\")\nwater()\nfertilize()\nharvest()\nmove(\"right\")", ""},
        {20, "自动果园总控", "最终关：大规模、多目标、长周期", {8, 6}, 480, {{"sunflower", 10, 0}, {"corn", 10, 0}, {"tomato", 12, 0}, {"berry", 8, 0}}, funcs(allFuncs), {"最终综合"}, "till()\nplant(\"sunflower\")\nfertilize()\nwater()\ndebug()\nharvest()\nmove(\"right\")", "bugs"},
    };
}
