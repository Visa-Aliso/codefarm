#include "savemanager.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QStandardPaths>

SaveManager::SaveManager(QObject *parent) : QObject(parent) {}

QString SaveManager::savePath() const {
    return QDir::homePath() + "/.codefarm/save.json";
}

QString SaveManager::scriptsPath() const {
    return QDir::homePath() + "/.codefarm/scripts/";
}

void SaveManager::save() {
    QDir().mkpath(QDir::homePath() + "/.codefarm");
    QFile f(savePath());
    if (f.open(QIODevice::WriteOnly)) {
        QJsonObject root;
        root["version"] = "1.0";
        root["levels"] = levelData_;
        f.write(QJsonDocument(root).toJson());
    }
}

void SaveManager::load() {
    QFile f(savePath());
    if (f.open(QIODevice::ReadOnly)) {
        auto doc = QJsonDocument::fromJson(f.readAll());
        levelData_ = doc.object().value("levels").toObject();
    }
}

QString SaveManager::loadScript(int levelId) const {
    QFile f(scriptsPath() + QString("level_%1.py").arg(levelId));
    if (f.open(QIODevice::ReadOnly))
        return QString::fromUtf8(f.readAll());
    return {};
}

void SaveManager::saveScript(int levelId, const QString &code) {
    QDir().mkpath(scriptsPath());
    QFile f(scriptsPath() + QString("level_%1.py").arg(levelId));
    if (f.open(QIODevice::WriteOnly))
        f.write(code.toUtf8());
}

void SaveManager::setLevelProgress(int id, int stars, int bestTime, int clearCount) {
    QJsonObject lev;
    lev["stars"] = stars;
    lev["bestTime"] = bestTime;
    lev["clearCount"] = clearCount;
    levelData_[QString::number(id)] = lev;
    save();
}
