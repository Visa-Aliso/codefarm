#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include <QObject>
#include <QJsonObject>

class SaveManager : public QObject {
    Q_OBJECT
public:
    explicit SaveManager(QObject *parent = nullptr);

    Q_INVOKABLE void save();
    Q_INVOKABLE void load();
    Q_INVOKABLE QString loadScript(int levelId) const;
    Q_INVOKABLE void saveScript(int levelId, const QString &code);

    QJsonObject levelProgress() const { return levelData_; }
    void setLevelProgress(int id, int stars, int bestTime, int clearCount);

private:
    QString savePath() const;
    QString scriptsPath() const;
    QJsonObject levelData_;
};

#endif // SAVEMANAGER_H
