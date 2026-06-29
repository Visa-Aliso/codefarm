#ifndef TECHTREE_H
#define TECHTREE_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QSet>

enum class TechBranch { A_Agriculture, B_Mechanics, C_Programming, D_Land };

struct TechNode {
    QString id;
    TechBranch branch;
    QString name;
    int cost;
    QVector<QString> prerequisites;
    QString effect;
};

class TechTree : public QObject {
    Q_OBJECT
public:
    explicit TechTree(QObject *parent = nullptr);

    const QVector<TechNode> &allNodes() const { return m_nodes; }
    bool isUnlocked(const QString &id) const;
    bool canUnlock(const QString &id) const;
    bool unlock(const QString &id); // returns true if newly unlocked

    QSet<QString> unlockedIds() const { return m_unlocked; }
    void setUnlocked(const QSet<QString> &ids) { m_unlocked = ids; }

    // Effects queries
    double yieldMultiplier() const;
    double diseaseResistBonus() const;
    double droneSpeedBonus() const;
    int extraDrones() const;
    bool hasSmartPathfinding() const;
    double cultivationSpeedMultiplier() const;
    double fertilityCapBonus() const;

signals:
    void nodeUnlocked(const QString &id);

private:
    void buildGraph();
    QVector<TechNode> m_nodes;
    QSet<QString> m_unlocked;
};

#endif // TECHTREE_H
