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

void GoalSystem::completeFeatureGoal() {
    for (int i = 0; i < goals_.size(); i++) {
        if (goals_[i].type == GoalType::Custom && !goals_[i].completed) {
            goals_[i].currentValue = 1;
            goals_[i].completed = true;
            emit goalCompleted(i);
        }
    }
}

void GoalSystem::checkAll(int timeElapsed) {
    for (auto &g : goals_) {
        if (g.type == GoalType::TimeLimit && !g.completed) {
            g.currentValue = timeElapsed;
        }
    }
    if (allRequiredCompleted()) {
        emit allGoalsCompleted(starsEarned());
    }
}

void GoalSystem::checkTick(int tickCount) {
    for (auto &g : goals_) {
        if (g.type == GoalType::TickLimit && !g.completed) {
            g.currentValue = tickCount;
            if (g.currentValue <= g.targetValue) {
                // tick 类目标在 finalizeTickGoals 时定稿，这里只更新当前值
            }
        }
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

void GoalSystem::finalizeTickGoals(int tickCount) {
    for (int i = 0; i < goals_.size(); i++) {
        auto &g = goals_[i];
        if (g.type == GoalType::TickLimit && !g.completed) {
            if (tickCount <= g.targetValue) {
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
    bool s1 = false, s2 = false, s3 = false;
    for (const auto &g : goals_) {
        if (g.completed) {
            if (g.starTier == StarTier::Star1) s1 = true;
            else if (g.starTier == StarTier::Star2) s2 = true;
            else if (g.starTier == StarTier::Star3) s3 = true;
        }
    }
    return (s1 ? 1 : 0) + (s2 ? 1 : 0) + (s3 ? 1 : 0);
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
