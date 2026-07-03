#pragma once

#include "Entity/Entity.h"
#include "Entity/BuildingTypes.h"
#include "Resource/ResourceCost.h"

#include <SFML/Graphics/Color.hpp>

#include <string>
#include <vector>

class Building : public Entity {
public:
    static constexpr float BUILD_TIME = 3.0f;

    explicit Building(BuildingType type);

    BuildingType getBuildingType() const { return m_type; }

    bool isBuilt() const { return m_buildProgress >= BUILD_TIME; }
    float getBuildProgress() const { return m_buildProgress; }
    void addBuildProgress(float amount);

    ResourceCost getCost() const;
    sf::Vector2i getSize() const;
    std::vector<sf::Vector2i> getOccupiedTiles() const;
    std::string getName() const;
    sf::Color getColor() const;
    float getProductionRate(ResourceType type) const;

    // Combat
    int getHP() const { return m_hp; }
    int getMaxHP() const { return m_maxHp; }
    bool isAlive() const { return m_hp > 0; }
    void takeDamage(int dmg) { m_hp = std::max(0, m_hp - dmg); }

    void update(float dt) override;
    void render(sf::RenderWindow& window, const sf::Vector2f& worldPos, int tileSize) override;

private:
    BuildingType m_type;
    float m_buildProgress = 0.0f;
    int m_hp = 0;
    int m_maxHp = 0;
};
