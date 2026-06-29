#include "SaveManager.h"
#include "core/GameState.h"

bool SaveManager::save(GameState *state, const QString &path,
                       const QByteArray &windowLayout) {
    return state->saveToFile(path, windowLayout);
}

bool SaveManager::load(GameState *state, const QString &path,
                       QByteArray &windowLayout) {
    if (!state->loadFromFile(path)) return false;
    windowLayout = state->savedWindowLayout();
    return true;
}
