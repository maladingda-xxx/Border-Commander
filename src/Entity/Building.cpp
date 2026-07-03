#include "Entity/Building.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include <algorithm>

namespace {

struct BuildingInfo {
    std::string name;
    sf::Color color;
    int width;
    int height;
    int goldCost;
    int foodCost;
    float goldProduction;
    float foodProduction;
};

BuildingInfo getBuildingInfo(BuildingType type) {
    switch (type) {
    case BuildingType::Headquarters:
        return {"HQ", sf::Color(255, 215, 0), 2, 2, 0, 0, 0.0f, 0.0f};
    case BuildingType::Barracks:
        return {"Barracks", sf::Color(220, 60, 60), 1, 1, 100, 20, 0.0f, 0.0f};
    case BuildingType::Farm:
        return {"Farm", sf::Color(154, 205, 50), 1, 1, 50, 0, 0.0f, 2.0f};
    case BuildingType::GoldMine:
        return {"Mine", sf::Color(184, 134, 11), 1, 1, 50, 0, 1.0f, 0.0f};
    }
    return {};
}

} // anonymous namespace

Building::Building(BuildingType type)
    : m_type(type) {
    if (type == BuildingType::Headquarters) {
        m_buildProgress = BUILD_TIME; // HQ starts fully built
    }
}

void Building::addBuildProgress(float amount) {
    m_buildProgress = std::min(m_buildProgress + amount, BUILD_TIME);
}

ResourceCost Building::getCost() const {
    ResourceCost cost;
    auto info = getBuildingInfo(m_type);
    if (info.goldCost > 0) {
        cost.set(ResourceType::Gold, info.goldCost);
    }
    if (info.foodCost > 0) {
        cost.set(ResourceType::Food, info.foodCost);
    }
    return cost;
}

sf::Vector2i Building::getSize() const {
    auto info = getBuildingInfo(m_type);
    return {info.width, info.height};
}

std::vector<sf::Vector2i> Building::getOccupiedTiles() const {
    std::vector<sf::Vector2i> tiles;
    auto pos = getTilePosition();
    auto size = getSize();
    for (int dy = 0; dy < size.y; ++dy) {
        for (int dx = 0; dx < size.x; ++dx) {
            tiles.push_back({pos.x + dx, pos.y + dy});
        }
    }
    return tiles;
}

std::string Building::getName() const {
    return getBuildingInfo(m_type).name;
}

sf::Color Building::getColor() const {
    auto info = getBuildingInfo(m_type);
    if (!isBuilt()) {
        return sf::Color(info.color.r / 2, info.color.g / 2, info.color.b / 2);
    }
    return info.color;
}

float Building::getProductionRate(ResourceType type) const {
    if (!isBuilt()) {
        return 0.0f;
    }
    auto info = getBuildingInfo(m_type);
    switch (type) {
    case ResourceType::Gold:
        return info.goldProduction;
    case ResourceType::Food:
        return info.foodProduction;
    default:
        return 0.0f;
    }
}

void Building::update(float dt) {
    if (!isBuilt()) {
        addBuildProgress(dt);
    }
}

void Building::render(sf::RenderWindow& window, const sf::Vector2f& worldPos, int tileSize) {
    auto size = getSize();
    auto color = getColor();

    sf::RectangleShape shape({
        static_cast<float>(size.x * tileSize),
        static_cast<float>(size.y * tileSize)
    });
    shape.setPosition(worldPos);
    shape.setFillColor(color);

    // Draw a darker border
    shape.setOutlineColor(sf::Color(40, 40, 40));
    shape.setOutlineThickness(1.0f);

    window.draw(shape);

    if (!isBuilt()) {
        float progress = m_buildProgress / BUILD_TIME;
        sf::RectangleShape bar({
            static_cast<float>(size.x * tileSize) * progress,
            4.0f
        });
        bar.setPosition({worldPos.x, worldPos.y + size.y * tileSize - 4.0f});
        bar.setFillColor(sf::Color::White);
        window.draw(bar);
    }
}
