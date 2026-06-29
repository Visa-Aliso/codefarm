#pragma once

#include <QString>
#include <QSize>
#include <QVector>
#include <QStringList>

struct CropInfo {
    QString key;
    QString name;
    QString icon;
    int ticks;
    double minWater;
    double bugRate;
};

struct Goal {
    QString crop;
    int target = 0;
    int done = 0;
};

struct Level {
    int id = 1;
    QString name;
    QString theme;
    QSize size;
    int seconds = 90;
    QVector<Goal> goals;
    QStringList functions;
    QStringList syntax;
    QString starter;
    QString setup;
};

struct Cell {
    bool tilled = false;
    QString crop;
    double progress = 0;
    double water = 0;
    int fertilizer = 0;
    bool bug = false;
};
