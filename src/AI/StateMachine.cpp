#include "AI/StateMachine.h"
#include "AI/IState.h"

StateMachine::StateMachine(Unit* owner)
    : m_owner(owner) {
}

void StateMachine::update(float dt) {
    if (m_currentState) {
        m_currentState->execute(m_owner, dt);
    }
}

void StateMachine::changeState(std::unique_ptr<IState> newState) {
    if (m_currentState) {
        m_currentState->exit(m_owner);
    }
    m_currentState = std::move(newState);
    if (m_currentState) {
        m_currentState->enter(m_owner);
    }
}

std::string StateMachine::getCurrentStateName() const {
    if (m_currentState) {
        return m_currentState->getName();
    }
    return "None";
}
