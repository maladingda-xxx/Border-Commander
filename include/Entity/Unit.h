#pragma once

#include "Entity/Entity.h"

#include <SFML/Graphics/Color.hpp>

#include <memory>
#include <vector>

class Entity;
struct ResourceCost;
class StateMachine;

enum class Faction {
    Player,
    Enemy
};

class Unit : public Entity {
public:
    explicit Unit(Faction faction);
    ~Unit() override = default;

    // Faction
    Faction getFaction() const { return m_faction; }

    // Movement
    void setTarget(const sf::Vector2i& tile);
    void stop();
    bool isMoving() const { return m_moving; }
    sf::Vector2i getTargetTile() const { return m_targetTile; }
    sf::Vector2f getWorldPos() const { return m_worldPos; }
    void setWorldPos(const sf::Vector2f& pos) { m_worldPos = pos; }

    // Combat
    bool isAlive() const { return m_hp > 0; }
    void takeDamage(int dmg);
    int getHP() const { return m_hp; }
    int getMaxHP() const { return m_maxHp; }
    int getAttack() const { return m_attack; }
    int getDefense() const { return m_defense; }

    // AI
    StateMachine* getStateMachine() { return m_stateMachine.get(); }
    void setEntityList(const std::vector<std::unique_ptr<Entity>>* list) { m_entities = list; }
    const std::vector<std::unique_ptr<Entity>>* getEntityList() const { return m_entities; }
    Entity* findNearestHostile(float range) const;
    sf::Vector2i worldToTile(const sf::Vector2f& worldPos) const;
    void commandMoveTo(const sf::Vector2i& tile);

    void update(float dt) override;
    void render(sf::RenderWindow& window, const sf::Vector2f& worldPos, int tileSize) override;

protected:
    int m_hp = 50;
    int m_maxHp = 50;
    int m_attack = 10;
    int m_defense = 5;
    float m_speed = 3.0f;

protected:
    Faction m_faction;
    sf::Vector2f m_worldPos;
    sf::Vector2i m_targetTile;
    bool m_moving = false;

    std::unique_ptr<StateMachine> m_stateMachine;
    const std::vector<std::unique_ptr<Entity>>* m_entities = nullptr;
};

class Soldier : public Unit {
public:
    Soldier();
    static ResourceCost getRecruitCost();
};

class Enemy : public Unit {
public:
    Enemy(int wave, const sf::Vector2i& targetTile);
    int getWaveNumber() const { return m_waveNumber; }
    int getBounty() const { return m_bounty; }

private:
    int m_waveNumber;
    int m_bounty;
};
