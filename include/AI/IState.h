#pragma once

#include <string>

class Unit;

class IState {
public:
    virtual ~IState() = default;

    virtual void enter(Unit* unit) = 0;
    virtual void execute(Unit* unit, float dt) = 0;
    virtual void exit(Unit* unit) = 0;

    virtual std::string getName() const = 0;
};
