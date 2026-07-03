#pragma once

#include <memory>
#include <string>

class IState;
class Unit;

class StateMachine {
public:
    explicit StateMachine(Unit* owner);

    void update(float dt);
    void changeState(std::unique_ptr<IState> newState);

    std::string getCurrentStateName() const;

private:
    Unit* m_owner;
    std::unique_ptr<IState> m_currentState;
};
