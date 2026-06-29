#ifndef SCRIPTMANAGER_H
#define SCRIPTMANAGER_H

#include <QObject>
#include <QVector>

class GameState;

struct ScriptInfo {
    QString name;
    QString code;
    bool running = false;
};

class ScriptManager : public QObject {
    Q_OBJECT
public:
    explicit ScriptManager(GameState *state, QObject *parent = nullptr);

    void addScript(const QString &name, const QString &code);
    void removeScript(int index);
    QVector<ScriptInfo> &scripts() { return m_scripts; }

signals:
    void scriptListChanged();

private:
    GameState *m_gameState;
    QVector<ScriptInfo> m_scripts;
};

#endif // SCRIPTMANAGER_H
