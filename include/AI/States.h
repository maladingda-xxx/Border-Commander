#pragma once

#include "AI/IState.h"
#include "World/TileMap.h"

#include <SFML/System/Vector2.hpp>

class Entity;

class IdleState : public IState {
public:
    void enter(Unit* unit) override;
    void execute(Unit* unit, float dt) override;
    void exit(Unit* unit) override;
    std::string getName() const override { return "Idle"; }

private:
    static constexpr float DETECTION_RANGE = 5.0f;
};

class MoveToState : public IState {
public:
    explicit MoveToState(const sf::Vector2i& target);

    void enter(Unit* unit) override;
    void execute(Unit* unit, float dt) override;
    void exit(Unit* unit) override;
    std::string getName() const override { return "MoveTo"; }

private:
    sf::Vector2i m_target;
    static constexpr float AMBUSH_RANGE = 3.0f;
};

class CombatState : public IState {
public:
    CombatState();

    void enter(Unit* unit) override;
    void execute(Unit* unit, float dt) override;
    void exit(Unit* unit) override;
    std::string getName() const override { return "Combat"; }

private:
    Entity* m_target = nullptr;
    float m_attackTimer = 0.0f;

    static constexpr float CHASE_RANGE = 5.0f;
    static constexpr float ATTACK_RANGE = 1.5f;
    static constexpr float ATTACK_INTERVAL = 1.0f;
    static constexpr int TILE_SIZE = TileMap::TILE_SIZE;

    sf::Vector2f getEntityCenter(const Entity* entity) const;
    int resolveDamage(const Unit* attacker, const Entity* target) const;
};
