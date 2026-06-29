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
    // assign replaces all cells with fresh defaults — prevents stale state
    // from a previous level (larger grid, planted crops, bugs, etc.) leaking through
    cells_.assign(w * h, Cell());
    pestZone_.assign(w * h, false);
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            cells_[y * w + x].x = x;
            cells_[y * w + x].y = y;
        }
    for (const auto &p : preset) {
        if (p.x >= 0 && p.x < w && p.y >= 0 && p.y < h) {
            cells_[p.y * w + p.x] = p;
            // 标记虫害区（用 bugImmuneTicks == -2 标记）
            if (p.bugImmuneTicks == -2) {
                pestZone_[p.y * w + p.x] = true;
                cells_[p.y * w + p.x].bugImmuneTicks = 0;
            }
            // 标记岩石（用 tilledTicks == -1 标记）
            if (p.tilledTicks == -1) {
                cells_[p.y * w + p.x].state = CellState::Rock;
                cells_[p.y * w + p.x].tilledTicks = 0;
            }
        }
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

float FarmMap::sunflowerAdjacency(int x, int y) const {
    float bonus = 0.0f;
    // 检查周围8个方向是否有向日葵
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && nx < gridW_ && ny >= 0 && ny < gridH_) {
                const Cell &neighbor = cells_[ny * gridW_ + nx];
                bonus += neighbor.adjacencyBonus();
            }
        }
    }
    return bonus;
}

void FarmMap::tickUpdate(float bugProbability) {
    for (int i = 0; i < cells_.size(); i++) {
        Cell &c = cells_[i];

        // 岩石和空地跳过
        if (c.state == CellState::Empty || c.state == CellState::Rock) {
            continue;
        }

        bool changed = false;

        // 水分自然蒸发
        if (c.water > 0) {
            const float nextWater = qMax(0.0f, c.water - 0.01f);
            changed = changed || !qFuzzyCompare(c.water, nextWater);
            c.water = nextWater;
        }
        // 施肥倒计时
        if (c.fertilizeTicks > 0) {
            c.fertilizeTicks--;
            changed = true;
            if (c.fertilizeTicks == 0) {
                c.fertilized = false;
            }
        }
        // 虫害免疫倒计时
        if (c.bugImmuneTicks > 0) {
            c.bugImmuneTicks--;
        }

        // 虫害生成（虫害区概率翻倍）
        const bool cropCell = c.crop != CropType::None &&
                              (c.state == CellState::Planted || c.state == CellState::Mature);
        if (cropCell && !c.hasBug && c.bugImmuneTicks == 0 && bugProbability > 0.0f) {
            float spawnRate = bugProbability;
            // 虫害区概率翻倍
            if (i < pestZone_.size() && pestZone_[i]) {
                spawnRate *= 2.0f;
            }
            if (QRandomGenerator::global()->generateDouble() < spawnRate) {
                c.hasBug = true;
                c.bugTicks = 0;
                changed = true;
            }
        }

        // 虫害效果：加速水分流失
        if (c.hasBug) {
            c.bugTicks++;
            const float bugWater = qMax(0.0f, c.water - 0.03f);
            changed = changed || !qFuzzyCompare(c.water, bugWater);
            c.water = bugWater;
        } else {
            c.bugTicks = 0;
        }

        // 作物生长
        if (c.state == CellState::Planted) {
            float baseSpeed = 1.0f / c.growthTicks();
            float wMul = (c.water >= c.waterThreshold()) ? 1.0f :
                         (c.water > 0 ? 0.5f : 0.0f);
            float fMul = c.fertilized ? (c.water >= 0.6f ? 1.8f : 1.5f) : 1.0f;

            // 向日葵阳光加成
            float adjMul = 1.0f + sunflowerAdjacency(c.x, c.y);

            if (c.hasBug) {
                wMul = 0.0f;
            }

            const float growth = baseSpeed * wMul * fMul * adjMul;
            if (growth > 0.0f) {
                c.progress = qMin(1.0f, c.progress + growth);
                changed = true;
            }

            // 虫害导致进度倒退
            if (c.hasBug && c.bugTicks % 8 == 0 && c.progress > 0.0f) {
                c.progress = qMax(0.0f, c.progress - 0.02f);
                changed = true;
            }

            // 旱灾惩罚：敏感作物在缺水时损失进度
            if (c.isDroughtSensitive() && c.water <= 0.0f && c.progress > 0.0f) {
                c.progress = qMax(0.0f, c.progress - c.droughtPenalty());
                changed = true;
            }

            // 过涝惩罚：敏感作物在水过多时损失进度
            if (c.isOverwaterSensitive() && c.water > 0.85f && c.progress > 0.0f) {
                c.progress = qMax(0.0f, c.progress - c.overwaterPenalty());
                changed = true;
            }

            // 成熟判定（向日葵不可收割，保持 Planted 状态）
            if (c.progress >= 1.0f) {
                c.progress = 1.0f;
                if (c.isHarvestable()) {
                    c.state = CellState::Mature;
                }
                // 向日葵保持 Planted，progress=1.0 持续提供加成
                changed = true;
            }
        }

        if (changed) {
            emit dataChanged(index(i), index(i));
            emit cellChanged(c.x, c.y);
        }
    }
}
