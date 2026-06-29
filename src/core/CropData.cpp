#include "CropData.h"
#include <stdexcept>

static const std::array<CropDef, 5> s_crops = {{
    {CropType::Apple,       "草莓",   18.0, 14, 0.18, 0.04, 0.25, true},
    {CropType::Grape,       "番茄",   28.0, 28, 0.35, 0.10, 0.30, false},
    {CropType::Banana,      "葡萄",   38.0, 46, 0.30, 0.26, 0.20, false},
    {CropType::Watermelon,  "西瓜",   48.0, 72, 0.70, 0.18, 0.18, true},
    {CropType::Pineapple,   "菠萝",   70.0, 120, 0.42, 0.16, 0.70, true},
}};

const std::array<CropDef, 5> &CropData::all() { return s_crops; }

const CropDef &CropData::get(CropType type) {
    for (auto &c : s_crops)
        if (c.type == type) return c;
    throw std::runtime_error("Invalid crop type");
}

int CropData::count() { return static_cast<int>(s_crops.size()); }
