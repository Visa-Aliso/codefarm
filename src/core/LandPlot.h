#ifndef LANDPLOT_H
#define LANDPLOT_H

#include <QString>

enum class CropType { None = -1, Apple = 0, Grape = 1, Banana = 2, Watermelon = 3, Pineapple = 4 };
enum class LandState { Uncultivated, Empty, Planted, Mature, Harvestable, Wasteland, Bugged };

inline QString cropName(CropType c) {
    switch (c) {
    case CropType::Apple: return QStringLiteral("草莓");
    case CropType::Grape: return QStringLiteral("番茄");
    case CropType::Banana: return QStringLiteral("葡萄");
    case CropType::Watermelon: return QStringLiteral("西瓜");
    case CropType::Pineapple: return QStringLiteral("菠萝");
    default: return QStringLiteral("无");
    }
}

inline QString landStateName(LandState s) {
    switch (s) {
    case LandState::Uncultivated: return QStringLiteral("未开垦");
    case LandState::Empty: return QStringLiteral("空地");
    case LandState::Planted: return QStringLiteral("种植中");
    case LandState::Mature: return QStringLiteral("成熟中");
    case LandState::Harvestable: return QStringLiteral("可收获");
    case LandState::Wasteland: return QStringLiteral("荒地");
    case LandState::Bugged: return QStringLiteral("病害");
    default: return QStringLiteral("未知");
    }
}

struct LandPlot {
    double water = 100.0;
    double fertility = 100.0;
    CropType crop = CropType::None;
    double progress = 0.0;       // 0.0–100.0
    bool bug = false;
    LandState state = LandState::Uncultivated;
};

#endif // LANDPLOT_H
