#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include <QString>
#include <QJsonObject>

class GameState;

class SaveManager {
public:
    static bool save(GameState *state, const QString &path,
                     const QByteArray &windowLayout);
    static bool load(GameState *state, const QString &path,
                     QByteArray &windowLayout);
};

#endif // SAVEMANAGER_H
