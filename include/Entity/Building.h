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

    void update(float dt) override;
    void render(sf::RenderWindow& window, const sf::Vector2f& worldPos, int tileSize) override;

private:
    BuildingType m_type;
    float m_buildProgress = 0.0f;
};
