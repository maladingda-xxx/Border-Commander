#include "Entity/Unit.h"
#include "Resource/ResourceCost.h"
#include "World/TileMap.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include <algorithm>
#include <cmath>

// ─── Unit ────────────────────────────────────────────────────────────

Unit::Unit(Faction faction)
    : m_faction(faction)
    , m_worldPos(0.0f, 0.0f) {
}

void Unit::setTarget(const sf::Vector2i& tile) {
    m_targetTile = tile;

    sf::Vector2f targetCenter(
        static_cast<float>(tile.x * TileMap::TILE_SIZE + TileMap::TILE_SIZE / 2),
        static_cast<float>(tile.y * TileMap::TILE_SIZE + TileMap::TILE_SIZE / 2)
    );

    float dx = targetCenter.x - m_worldPos.x;
    float dy = targetCenter.y - m_worldPos.y;
    if (std::abs(dx) < 0.5f && std::abs(dy) < 0.5f) {
        return;
    }

    m_moving = true;
}

void Unit::stop() {
    m_moving = false;
}

void Unit::takeDamage(int dmg) {
    int actualDmg = std::max(1, dmg - m_defense);
    m_hp = std::max(0, m_hp - actualDmg);
}

void Unit::update(float dt) {
    if (!m_moving) {
        return;
    }

    sf::Vector2f targetCenter(
        static_cast<float>(m_targetTile.x * TileMap::TILE_SIZE + TileMap::TILE_SIZE / 2),
        static_cast<float>(m_targetTile.y * TileMap::TILE_SIZE + TileMap::TILE_SIZE / 2)
    );

    float dx = targetCenter.x - m_worldPos.x;
    float dy = targetCenter.y - m_worldPos.y;
    float dist = std::sqrt(dx * dx + dy * dy);

    float moveAmount = m_speed * dt * TileMap::TILE_SIZE;

    if (dist <= moveAmount) {
        m_worldPos = targetCenter;
        m_moving = false;
    } else {
        m_worldPos.x += (dx / dist) * moveAmount;
        m_worldPos.y += (dy / dist) * moveAmount;
    }

    setTilePosition({
        static_cast<int>(m_worldPos.x / TileMap::TILE_SIZE),
        static_cast<int>(m_worldPos.y / TileMap::TILE_SIZE)
    });
}

void Unit::render(sf::RenderWindow& window, const sf::Vector2f& /*worldPos*/, int tileSize) {
    sf::Color color;
    if (m_faction == Faction::Player) {
        color = sf::Color(60, 100, 255);
    } else {
        color = sf::Color(255, 60, 60);
    }

    // Use m_worldPos for rendering (smooth position)
    sf::Vector2f drawPos(
        m_worldPos.x - static_cast<float>(tileSize) / 2.0f,
        m_worldPos.y - static_cast<float>(tileSize) / 2.0f
    );

    sf::RectangleShape shape({
        static_cast<float>(tileSize),
        static_cast<float>(tileSize)
    });
    shape.setPosition(drawPos);
    shape.setFillColor(color);
    shape.setOutlineColor(sf::Color(30, 30, 30));
    shape.setOutlineThickness(1.0f);
    window.draw(shape);

    // HP bar
    if (m_hp < m_maxHp) {
        float hpPercent = static_cast<float>(m_hp) / static_cast<float>(m_maxHp);
        float barWidth = static_cast<float>(tileSize);
        float barHeight = 4.0f;

        sf::RectangleShape bg({barWidth, barHeight});
        bg.setPosition({drawPos.x, drawPos.y - barHeight - 2.0f});
        bg.setFillColor(sf::Color(40, 40, 40));
        window.draw(bg);

        sf::Color hpColor;
        if (hpPercent > 0.5f) {
            hpColor = sf::Color::Green;
        } else if (hpPercent > 0.25f) {
            hpColor = sf::Color::Yellow;
        } else {
            hpColor = sf::Color::Red;
        }

        sf::RectangleShape fill({barWidth * hpPercent, barHeight});
        fill.setPosition({drawPos.x, drawPos.y - barHeight - 2.0f});
        fill.setFillColor(hpColor);
        window.draw(fill);
    }
}

// ─── Soldier ─────────────────────────────────────────────────────────

Soldier::Soldier()
    : Unit(Faction::Player) {
}

ResourceCost Soldier::getRecruitCost() {
    ResourceCost cost;
    cost.set(ResourceType::Gold, 50);
    cost.set(ResourceType::Food, 10);
    return cost;
}

// ─── Enemy ───────────────────────────────────────────────────────────

Enemy::Enemy(int wave, const sf::Vector2i& targetTile)
    : Unit(Faction::Enemy)
    , m_waveNumber(wave)
    , m_bounty(25 + wave * 5) {
    setTarget(targetTile);
}
