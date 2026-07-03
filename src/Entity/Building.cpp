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
    int maxHp;
};

BuildingInfo getBuildingInfo(BuildingType type) {
    switch (type) {
    case BuildingType::Headquarters:
        return {"HQ", sf::Color(255, 215, 0), 2, 2, 0, 0, 0.0f, 0.0f, 500};
    case BuildingType::Barracks:
        return {"Barracks", sf::Color(220, 60, 60), 1, 1, 100, 20, 0.0f, 0.0f, 200};
    case BuildingType::Farm:
        return {"Farm", sf::Color(154, 205, 50), 1, 1, 50, 0, 0.0f, 2.0f, 150};
    case BuildingType::GoldMine:
        return {"Mine", sf::Color(184, 134, 11), 1, 1, 50, 0, 1.0f, 0.0f, 150};
    }
    return {};
}

} // anonymous namespace

Building::Building(BuildingType type)
    : m_type(type) {
    auto info = getBuildingInfo(type);
    m_hp = info.maxHp;
    m_maxHp = info.maxHp;
    if (type == BuildingType::Headquarters) {
        m_buildProgress = BUILD_TIME;
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

void Building::takeDamage(int dmg) {
    m_hp = std::max(0, m_hp - dmg);
    m_flashTimer = 0.1f;
}

void Building::update(float dt) {
    if (m_flashTimer > 0.0f) m_flashTimer -= dt;
    if (m_spawnTimer < 1.0f) m_spawnTimer += dt / 0.3f;
    if (!isBuilt()) {
        addBuildProgress(dt);
    }
}

void Building::render(sf::RenderWindow& window, const sf::Vector2f& worldPos, int tileSize) {
    auto size = getSize();
    auto color = getColor();

    float spawnScale = std::min(1.0f, 0.2f + m_spawnTimer * 0.8f);
    uint8_t alpha = static_cast<uint8_t>(std::min(1.0f, m_spawnTimer) * 255.0f);
    color.a = static_cast<uint8_t>(color.a * alpha / 255);

    float w = static_cast<float>(size.x * tileSize) * spawnScale;
    float h = static_cast<float>(size.y * tileSize) * spawnScale;
    float ox = (static_cast<float>(size.x * tileSize) - w) / 2.0f;
    float oy = (static_cast<float>(size.y * tileSize) - h) / 2.0f;

    sf::RectangleShape shape({w, h});
    shape.setPosition({worldPos.x + ox, worldPos.y + oy});
    shape.setFillColor(color);
    shape.setOutlineColor(sf::Color(40, 40, 40, alpha));
    shape.setOutlineThickness(1.0f);
    window.draw(shape);

    // Damage flash
    if (m_flashTimer > 0.0f) {
        sf::RectangleShape flash({w, h});
        flash.setPosition({worldPos.x + ox, worldPos.y + oy});
        flash.setFillColor(sf::Color(255, 255, 255,
            static_cast<uint8_t>(m_flashTimer * 10.0f * 255.0f)));
        window.draw(flash);
    }

    // Construction progress
    if (!isBuilt()) {
        float progress = m_buildProgress / BUILD_TIME;
        sf::RectangleShape bar({w * progress, 4.0f});
        bar.setPosition({worldPos.x + ox, worldPos.y + oy + h - 4.0f});
        bar.setFillColor(sf::Color(255, 255, 255, alpha));
        window.draw(bar);
    }

    // HP bar (always visible)
    float hpPct = static_cast<float>(m_hp) / static_cast<float>(m_maxHp);
    float barW = w;
    float barH = 4.0f;
    float barY = worldPos.y - barH - 2.0f;

    sf::RectangleShape hpBg({barW, barH});
    hpBg.setPosition({worldPos.x + ox, barY});
    hpBg.setFillColor(sf::Color(40, 40, 40, alpha));
    window.draw(hpBg);

    sf::Color hpColor;
    if (hpPct > 0.5f) hpColor = sf::Color::Green;
    else if (hpPct > 0.25f) hpColor = sf::Color::Yellow;
    else hpColor = sf::Color::Red;
    hpColor.a = alpha;

    sf::RectangleShape hpFill({barW * hpPct, barH});
    hpFill.setPosition({worldPos.x + ox, barY});
    hpFill.setFillColor(hpColor);
    window.draw(hpFill);
}
