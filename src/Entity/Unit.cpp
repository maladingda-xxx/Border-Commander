#include "Entity/Unit.h"
#include "AI/StateMachine.h"
#include "AI/States.h"
#include "Entity/Building.h"
#include "Event/EventBus.h"
#include "Event/Events.h"
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
    m_stateMachine = std::make_unique<StateMachine>(this);
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
    m_hp = std::max(0, m_hp - dmg);
    m_flashTimer = 0.1f;

    if (m_eventBus) {
        m_eventBus->emit(Event::CombatEvent{-1, getId(), dmg});
    }
}

void Unit::commandMoveTo(const sf::Vector2i& tile) {
    m_stateMachine->changeState(std::make_unique<MoveToState>(tile));
}

sf::Vector2i Unit::worldToTile(const sf::Vector2f& worldPos) const {
    return {
        static_cast<int>(worldPos.x / TileMap::TILE_SIZE),
        static_cast<int>(worldPos.y / TileMap::TILE_SIZE)
    };
}

Entity* Unit::findNearestHostile(float range) const {
    if (!m_entities) {
        return nullptr;
    }

    sf::Vector2f myPos = m_worldPos;
    Entity* nearest = nullptr;
    float nearestDist = range * static_cast<float>(TileMap::TILE_SIZE);

    for (auto& entity : *m_entities) {
        if (!entity->isActive() || entity.get() == this) {
            continue;
        }

        bool hostile = false;
        if (auto* otherUnit = dynamic_cast<Unit*>(entity.get())) {
            hostile = (m_faction != otherUnit->getFaction());
        } else if (dynamic_cast<Building*>(entity.get())) {
            // Enemies are hostile to all player buildings
            hostile = (m_faction == Faction::Enemy);
        }

        if (!hostile) {
            continue;
        }

        // Get entity position
        sf::Vector2f otherPos;
        if (auto* otherUnit = dynamic_cast<Unit*>(entity.get())) {
            otherPos = otherUnit->getWorldPos();
        } else {
            // Building center
            otherPos = {
                static_cast<float>(entity->getTilePosition().x * TileMap::TILE_SIZE + TileMap::TILE_SIZE / 2),
                static_cast<float>(entity->getTilePosition().y * TileMap::TILE_SIZE + TileMap::TILE_SIZE / 2)
            };
        }

        float dx = otherPos.x - myPos.x;
        float dy = otherPos.y - myPos.y;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist < nearestDist) {
            nearestDist = dist;
            nearest = entity.get();
        }
    }

    return nearest;
}

void Unit::update(float dt) {
    if (m_flashTimer > 0.0f) m_flashTimer -= dt;
    if (m_spawnTimer < 1.0f) m_spawnTimer += dt / 0.3f;
    m_bobTimer += dt;

    if (m_stateMachine) {
        m_stateMachine->update(dt);
    }

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
    float spawnScale = std::min(1.0f, 0.2f + m_spawnTimer * 0.8f);
    uint8_t spawnAlpha = static_cast<uint8_t>(std::min(1.0f, m_spawnTimer) * 255.0f);

    sf::Color color;
    if (m_faction == Faction::Player) {
        color = sf::Color(60, 100, 255, spawnAlpha);
    } else {
        color = sf::Color(255, 60, 60, spawnAlpha);
    }

    float bobOffset = 0.0f;
    if (m_moving) {
        bobOffset = std::sin(m_bobTimer * 8.0f) * 3.0f;
    }

    float half = static_cast<float>(tileSize) * spawnScale / 2.0f;
    sf::Vector2f drawPos(
        m_worldPos.x - half,
        m_worldPos.y - half + bobOffset
    );

    sf::RectangleShape shape({
        static_cast<float>(tileSize) * spawnScale,
        static_cast<float>(tileSize) * spawnScale
    });
    shape.setPosition(drawPos);
    shape.setFillColor(color);
    shape.setOutlineColor(sf::Color(30, 30, 30, spawnAlpha));
    shape.setOutlineThickness(1.0f);
    window.draw(shape);

    // Damage flash
    if (m_flashTimer > 0.0f) {
        sf::RectangleShape flash({shape.getSize().x, shape.getSize().y});
        flash.setPosition(drawPos);
        flash.setFillColor(sf::Color(255, 255, 255,
            static_cast<uint8_t>(m_flashTimer * 10.0f * 255.0f)));
        window.draw(flash);
    }

    // HP bar (always visible)
    float hpPct = static_cast<float>(m_hp) / static_cast<float>(m_maxHp);
    float barW = static_cast<float>(tileSize);
    float barH = 4.0f;
    float barY = drawPos.y - barH - 2.0f;

    sf::RectangleShape hpBg({barW, barH});
    hpBg.setPosition({drawPos.x + (static_cast<float>(tileSize) * spawnScale - barW) / 2.0f, barY});
    hpBg.setFillColor(sf::Color(40, 40, 40, spawnAlpha));
    window.draw(hpBg);

    sf::Color hpColor;
    if (hpPct > 0.5f) hpColor = sf::Color::Green;
    else if (hpPct > 0.25f) hpColor = sf::Color::Yellow;
    else hpColor = sf::Color::Red;
    hpColor.a = spawnAlpha;

    sf::RectangleShape hpFill({barW * hpPct, barH});
    hpFill.setPosition({drawPos.x + (static_cast<float>(tileSize) * spawnScale - barW) / 2.0f, barY});
    hpFill.setFillColor(hpColor);
    window.draw(hpFill);
}

// ─── Soldier ─────────────────────────────────────────────────────────

Soldier::Soldier()
    : Unit(Faction::Player) {
    m_stateMachine->changeState(std::make_unique<IdleState>());
}

ResourceCost Soldier::getRecruitCost() {
    ResourceCost cost;
    cost.set(ResourceType::Gold, 40);
    cost.set(ResourceType::Food, 5);
    return cost;
}

// ─── Enemy ───────────────────────────────────────────────────────────

Enemy::Enemy(int wave, const sf::Vector2i& targetTile, int hpBonus, int atkBonus)
    : Unit(Faction::Enemy)
    , m_waveNumber(wave)
    , m_bounty(25 + wave * 5) {
    m_hp += hpBonus;
    m_maxHp += hpBonus;
    m_attack += atkBonus;
    m_stateMachine->changeState(std::make_unique<MoveToState>(targetTile));
}
