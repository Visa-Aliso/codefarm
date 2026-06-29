#include "goalsystem.h"

GoalSystem::GoalSystem(QObject *parent) : QObject(parent) {}

void GoalSystem::init(const QList<LevelGoal> &goals) {
    goals_ = goals;
}

void GoalSystem::reset() {
    for (auto &g : goals_) {
        g.currentValue = 0;
        g.completed = false;
    }
}

void GoalSystem::addHarvestCount(const QString &crop) {
    for (int i = 0; i < goals_.size(); i++) {
        auto &g = goals_[i];
        if (g.type == GoalType::HarvestCount && (g.cropType.isEmpty() || g.cropType == crop)) {
            g.currentValue++;
            if (g.currentValue >= g.targetValue && !g.completed) {
                g.completed = true;
                emit goalCompleted(i);
            }
        }
    }
}

void GoalSystem::checkAll(int timeElapsed) {
    Q_UNUSED(timeElapsed)
    if (allRequiredCompleted()) {
        emit allGoalsCompleted(starsEarned());
    }
}

void GoalSystem::finalizeTimeGoals(int timeElapsed) {
    for (int i = 0; i < goals_.size(); i++) {
        auto &g = goals_[i];
        if (g.type == GoalType::TimeLimit && !g.completed) {
            if (timeElapsed <= g.targetValue) {
                g.completed = true;
                emit goalCompleted(i);
            }
        }
    }
    if (allRequiredCompleted()) {
        emit allGoalsCompleted(starsEarned());
    }
}

bool GoalSystem::allRequiredCompleted() const {
    for (const auto &g : goals_) {
        if (g.starTier == StarTier::Star1 && !g.completed)
            return false;
    }
    return true;
}

int GoalSystem::starsEarned() const {
    int stars = 0;
    for (const auto &g : goals_) {
        if (g.completed)
            stars = qMax(stars, static_cast<int>(g.starTier));
    }
    return stars;
}

QVariantList GoalSystem::goalsModel() const {
    QVariantList list;
    for (const auto &g : goals_) {
        list.append(QVariantMap{
            {"description", g.description},
            {"current", g.currentValue},
            {"target", g.targetValue},
            {"completed", g.completed},
            {"starTier", static_cast<int>(g.starTier)}
        });
    }
    return list;
}
