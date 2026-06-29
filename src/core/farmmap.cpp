#include "farmmap.h"
#include <QRandomGenerator>

FarmMap::FarmMap(QObject *parent) : QAbstractListModel(parent) {}

int FarmMap::rowCount(const QModelIndex &) const {
    return cells_.size();
}

QVariant FarmMap::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= cells_.size())
        return {};
    const Cell &c = cells_[index.row()];
    switch (role) {
        case StateRole: return Cell::stateToString(c.state);
        case CropRole: return Cell::cropToString(c.crop);
        case ProgressRole: return c.progress;
        case WaterRole: return c.water;
        case FertilizedRole: return c.fertilized;
        case FertilizeTicksRole: return c.fertilizeTicks;
        case HasBugRole: return c.hasBug;
        case GridXRole: return c.x;
        case GridYRole: return c.y;
    }
    return {};
}

QHash<int, QByteArray> FarmMap::roleNames() const {
    return {
        {StateRole, "state"}, {CropRole, "crop"},
        {ProgressRole, "progress"}, {WaterRole, "water"},
        {FertilizedRole, "fertilized"}, {FertilizeTicksRole, "fertilizeTicks"},
        {HasBugRole, "hasBug"}, {GridXRole, "gridX"}, {GridYRole, "gridY"}
    };
}

QVariantMap FarmMap::getCellAt(int x, int y) const {
    if (x < 0 || x >= gridW_ || y < 0 || y >= gridH_) return {};
    const Cell &c = cells_[y * gridW_ + x];
    return {
        {"state", Cell::stateToString(c.state)},
        {"crop", Cell::cropToString(c.crop)},
        {"progress", c.progress},
        {"water", c.water},
        {"fertilized", c.fertilized},
        {"hasBug", c.hasBug}
    };
}

void FarmMap::init(int w, int h, const QVector<Cell> &preset) {
    beginResetModel();
    gridW_ = w; gridH_ = h;
    cells_.resize(w * h);
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            cells_[y * w + x].x = x;
            cells_[y * w + x].y = y;
        }
    for (const auto &p : preset) {
        if (p.x >= 0 && p.x < w && p.y >= 0 && p.y < h)
            cells_[p.y * w + p.x] = p;
    }
    presetCells_ = cells_;
    endResetModel();
    emit dimensionsChanged();
}

void FarmMap::resetToPreset() {
    beginResetModel();
    cells_ = presetCells_;
    endResetModel();
}

void FarmMap::notifyCellChanged(int x, int y) {
    if (x < 0 || x >= gridW_ || y < 0 || y >= gridH_) {
        return;
    }

    const QModelIndex modelIndex = index(y * gridW_ + x);
    emit dataChanged(modelIndex, modelIndex);
    emit cellChanged(x, y);
}

Cell& FarmMap::cellAt(int x, int y) { return cells_[y * gridW_ + x]; }
const Cell& FarmMap::cellAt(int x, int y) const { return cells_[y * gridW_ + x]; }

void FarmMap::tickUpdate(float bugProbability) {
    for (int i = 0; i < cells_.size(); i++) {
        Cell &c = cells_[i];
        if (c.state == CellState::Empty) {
            continue;
        }

        bool changed = false;

        if (c.water > 0) {
            const float nextWater = qMax(0.0f, c.water - 0.01f);
            changed = changed || !qFuzzyCompare(c.water, nextWater);
            c.water = nextWater;
        }
        if (c.fertilizeTicks > 0) {
            c.fertilizeTicks--;
            changed = true;
            if (c.fertilizeTicks == 0) {
                c.fertilized = false;
            }
        }
        if (c.bugImmuneTicks > 0) {
            c.bugImmuneTicks--;
        }

        const bool cropCell = c.crop != CropType::None &&
                              (c.state == CellState::Planted || c.state == CellState::Mature);
        if (cropCell && !c.hasBug && c.bugImmuneTicks == 0 && bugProbability > 0.0f &&
            QRandomGenerator::global()->generateDouble() < bugProbability) {
            c.hasBug = true;
            c.bugTicks = 0;
            changed = true;
        }

        if (c.hasBug) {
            c.bugTicks++;
            const float bugWater = qMax(0.0f, c.water - 0.03f);
            changed = changed || !qFuzzyCompare(c.water, bugWater);
            c.water = bugWater;
        } else {
            c.bugTicks = 0;
        }

        if (c.state == CellState::Planted) {
            float baseSpeed = 1.0f / c.growthTicks();
            float wMul = (c.water >= c.waterThreshold()) ? 1.0f :
                         (c.water > 0 ? 0.5f : 0.0f);
            float fMul = c.fertilized ? (c.water >= 0.6f ? 1.8f : 1.5f) : 1.0f;

            if (c.hasBug) {
                wMul = 0.0f;
            }

            const float growth = baseSpeed * wMul * fMul;
            if (growth > 0.0f) {
                c.progress = qMin(1.0f, c.progress + growth);
                changed = true;
            }

            if (c.hasBug && c.bugTicks % 8 == 0 && c.progress > 0.0f) {
                c.progress = qMax(0.0f, c.progress - 0.02f);
                changed = true;
            }

            if (c.progress >= 1.0f) {
                c.progress = 1.0f;
                c.state = CellState::Mature;
                changed = true;
            }
        }

        if (changed) {
            emit dataChanged(index(i), index(i));
            emit cellChanged(c.x, c.y);
        }
    }
}
