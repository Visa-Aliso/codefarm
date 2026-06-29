#ifndef CROPDATA_H
#define CROPDATA_H

#include "LandPlot.h"
#include <QString>
#include <array>

struct CropDef {
    CropType type;
    QString name;
    double growthTime;       // seconds to mature
    int yield;               // gold per harvest
    double waterUsePerSec;   // water consumed per second
    double fertUsePerSec;    // fertility consumed per second
    double bugResistance;    // 0–1, higher = more resistant
    bool replantAfterHarvest; // true = one-shot crops
};

class CropData {
public:
    static const std::array<CropDef, 5> &all();
    static const CropDef &get(CropType type);
    static int count();
};

#endif // CROPDATA_H
