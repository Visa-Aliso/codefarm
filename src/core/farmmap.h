#ifndef FARMMAP_H
#define FARMMAP_H

#include <QAbstractListModel>
#include <QVector>
#include <QVariantMap>
#include "cell.h"

class FarmMap : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int gridWidth READ gridWidth NOTIFY dimensionsChanged)
    Q_PROPERTY(int gridHeight READ gridHeight NOTIFY dimensionsChanged)

public:
    enum Roles {
        StateRole = Qt::UserRole + 1,
        CropRole,
        ProgressRole,
        WaterRole,
        FertilizedRole,
        FertilizeTicksRole,
        HasBugRole,
        GridXRole,
        GridYRole
    };

    explicit FarmMap(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int gridWidth() const { return gridW_; }
    int gridHeight() const { return gridH_; }

    Q_INVOKABLE QVariantMap getCellAt(int x, int y) const;

    void init(int w, int h, const QVector<Cell> &preset);
    void resetToPreset();
    void tickUpdate(float bugProbability);
    void notifyCellChanged(int x, int y);

    Cell& cellAt(int x, int y);
    const Cell& cellAt(int x, int y) const;

signals:
    void cellChanged(int x, int y);
    void dimensionsChanged();

private:
    float sunflowerAdjacency(int x, int y) const;

    int gridW_ = 0;
    int gridH_ = 0;
    QVector<Cell> cells_;
    QVector<Cell> presetCells_;
    QVector<bool> pestZone_;
};

#endif // FARMMAP_H
