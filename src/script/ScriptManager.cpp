#include "ScriptManager.h"
#include "core/GameState.h"

ScriptManager::ScriptManager(GameState *state, QObject *parent)
    : QObject(parent)
    , m_gameState(state)
{
    // Load scripts from game state
    for (auto &sd : m_gameState->scripts())
        m_scripts.append({sd.name, sd.code, false});
}

void ScriptManager::addScript(const QString &name, const QString &code) {
    m_scripts.append({name, code, false});
    m_gameState->scripts().append({name, code});
    emit scriptListChanged();
}

void ScriptManager::removeScript(int index) {
    if (index < 0 || index >= m_scripts.size()) return;
    m_scripts.removeAt(index);
    m_gameState->scripts().removeAt(index);
    emit scriptListChanged();
}
