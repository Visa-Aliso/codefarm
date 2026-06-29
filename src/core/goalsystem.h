#ifndef GOALSYSTEM_H
#define GOALSYSTEM_H

#include <QObject>
#include <QList>
#include <QVariantList>
#include "levels/leveldata.h"

class GoalSystem : public QObject {
    Q_OBJECT
public:
    explicit GoalSystem(QObject *parent = nullptr);

    void init(const QList<LevelGoal> &goals);
    void reset();
    void checkAll(int timeElapsed);
    void finalizeTimeGoals(int timeElapsed);
    void addHarvestCount(const QString &crop);
    bool allRequiredCompleted() const;
    int starsEarned() const;
    Q_INVOKABLE QVariantList goalsModel() const;

signals:
    void goalCompleted(int index);
    void allGoalsCompleted(int stars);

private:
    QList<LevelGoal> goals_;
};

#endif // GOALSYSTEM_H
