#ifndef CELL_H
#define CELL_H

#include <QString>

enum class CellState { Empty, Tilled, Planted, Mature, Bug, Rock };
enum class CropType { None, Wheat, Carrot, Tomato, Corn, Sunflower };

struct Cell {
    int x = 0;
    int y = 0;
    CellState state = CellState::Empty;
    CropType crop = CropType::None;
    float progress = 0.0f;
    float water = 0.0f;
    bool fertilized = false;
    int fertilizeTicks = 0;
    bool hasBug = false;
    int bugImmuneTicks = 0;
    int bugTicks = 0;
    int tilledTicks = 0;

    static QString stateToString(CellState s);
    static QString cropToString(CropType c);
    static CropType cropFromString(const QString &s);
    int growthTicks() const;
    float waterThreshold() const;
    bool isHarvestable() const;
    bool isDroughtSensitive() const;
    bool isOverwaterSensitive() const;
    float adjacencyBonus() const;
    float droughtPenalty() const;
    float overwaterPenalty() const;
};

#endif // CELL_H
