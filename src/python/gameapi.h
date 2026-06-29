#ifndef GAMEAPI_H
#define GAMEAPI_H

class FarmMap;
class Drone;
class GoalSystem;

struct GameAPIContext {
    FarmMap *map = nullptr;
    Drone *drone = nullptr;
    GoalSystem *goals = nullptr;
};

#endif // GAMEAPI_H
