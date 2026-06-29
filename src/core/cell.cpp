#include "cell.h"

QString Cell::stateToString(CellState s) {
    switch (s) {
        case CellState::Empty: return "empty";
        case CellState::Tilled: return "tilled";
        case CellState::Planted: return "planted";
        case CellState::Mature: return "mature";
        case CellState::Bug: return "bug";
        case CellState::Rock: return "rock";
    }
    return "empty";
}

QString Cell::cropToString(CropType c) {
    switch (c) {
        case CropType::None: return "";
        case CropType::Wheat: return "wheat";
        case CropType::Carrot: return "carrot";
        case CropType::Tomato: return "tomato";
        case CropType::Corn: return "corn";
        case CropType::Sunflower: return "sunflower";
    }
    return "";
}

CropType Cell::cropFromString(const QString &s) {
    if (s == "wheat") return CropType::Wheat;
    if (s == "carrot") return CropType::Carrot;
    if (s == "tomato") return CropType::Tomato;
    if (s == "corn") return CropType::Corn;
    if (s == "sunflower") return CropType::Sunflower;
    return CropType::None;
}

int Cell::growthTicks() const {
    switch (crop) {
        case CropType::Wheat: return 6;
        case CropType::Carrot: return 14;
        case CropType::Tomato: return 12;
        case CropType::Corn: return 20;
        case CropType::Sunflower: return 35;
        default: return 6;
    }
}

float Cell::waterThreshold() const {
    switch (crop) {
        case CropType::Wheat: return 0.0f;
        case CropType::Carrot: return 0.4f;
        case CropType::Tomato: return 0.25f;
        case CropType::Corn: return 0.2f;
        case CropType::Sunflower: return 0.3f;
        default: return 0.0f;
    }
}

bool Cell::isHarvestable() const {
    // 向日葵不可收割，仅用于阳光加成
    return crop != CropType::Sunflower;
}

bool Cell::isDroughtSensitive() const {
    // 玉米和向日葵在缺水时会损失进度
    return crop == CropType::Corn || crop == CropType::Sunflower;
}

bool Cell::isOverwaterSensitive() const {
    // 胡萝卜和向日葵在过涝时会损失进度
    return crop == CropType::Carrot || crop == CropType::Sunflower;
}

float Cell::adjacencyBonus() const {
    // 向日葵给周围格子提供生长加成
    if (crop == CropType::Sunflower) return 0.15f;
    return 0.0f;
}

float Cell::droughtPenalty() const {
    // 缺水时每tick损失的进度
    switch (crop) {
        case CropType::Corn: return 0.02f;
        case CropType::Sunflower: return 0.05f;
        default: return 0.0f;
    }
}

float Cell::overwaterPenalty() const {
    // 过涝时每tick损失的进度
    switch (crop) {
        case CropType::Carrot: return 0.03f;
        case CropType::Sunflower: return 0.05f;
        default: return 0.0f;
    }
}
