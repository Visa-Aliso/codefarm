#include "cell.h"

QString Cell::stateToString(CellState s) {
    switch (s) {
        case CellState::Empty: return "empty";
        case CellState::Tilled: return "tilled";
        case CellState::Planted: return "planted";
        case CellState::Mature: return "mature";
        case CellState::Bug: return "bug";
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
        case CropType::Wheat: return 20;
        case CropType::Carrot: return 30;
        case CropType::Tomato: return 25;
        case CropType::Corn: return 50;
        case CropType::Sunflower: return 90;
        default: return 20;
    }
}

float Cell::waterThreshold() const {
    switch (crop) {
        case CropType::Wheat: return 0.0f;
        case CropType::Carrot: return 0.4f;
        case CropType::Tomato: return 0.2f;
        case CropType::Corn: return 0.2f;
        case CropType::Sunflower: return 0.3f;
        default: return 0.0f;
    }
}
