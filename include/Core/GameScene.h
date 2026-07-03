#pragma once

#include "Core/Scene.h"
#include "Entity/BuildingTypes.h"
#include "Event/EventBus.h"
#include "Resource/ResourceManager.h"
#include "World/Camera.h"
#include "World/TileMap.h"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

#include <memory>
#include <vector>

class Building;
class Entity;
class GameClock;
class Unit;

class GameScene : public Scene {
public:
    explicit GameScene(const GameClock* clock);

    void onEnter() override;
    void onExit() override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
    void handleInput(const sf::Event& event) override;

private:
    const GameClock* m_clock;
    sf::Font m_font;
    std::unique_ptr<sf::Text> m_fpsText;
    std::unique_ptr<sf::Text> m_resourceText;
    std::unique_ptr<sf::Text> m_statusText;

    TileMap m_tileMap;
    Camera m_camera;
    bool m_dragging = false;
    sf::Vector2i m_lastMousePos;
    sf::Vector2i m_mouseDownPos;
    sf::Vector2i m_mouseScreenPos;
    sf::Vector2u m_windowSize{1024, 768};

    Event::EventBus m_eventBus;
    ResourceManager m_resourceManager;
    std::vector<std::unique_ptr<Entity>> m_entities;

    // Building placement
    bool m_placementMode = false;
    BuildingType m_selectedBuildingType = BuildingType::Headquarters;

    // Unit selection & enemy spawning
    int m_selectedEntityId = -1;
    float m_enemySpawnTimer = 0.0f;
    int m_waveCount = 0;
    static constexpr float ENEMY_SPAWN_INTERVAL = 10.0f;

    void placeInitialHeadquarters();
    void handleClick(const sf::Vector2i& screenPos);
    void handleEntityClick(const sf::Vector2i& screenPos);
    void handleRightClick(const sf::Vector2i& screenPos);
    void recalculateProduction();
    bool canPlaceBuilding(BuildingType type, const sf::Vector2i& tilePos);
    void placeBuilding(BuildingType type, const sf::Vector2i& tilePos);
    bool isTileOccupied(int x, int y) const;

    void tryRecruitSoldier(Building* barracks);
    void commandMove(const sf::Vector2i& tilePos);
    void spawnEnemy();
    Entity* findEntityAt(const sf::Vector2i& tilePos);
    sf::Vector2i findAdjacentEmptyTile(const sf::Vector2i& tilePos);
    bool isValidSpawnTile(int x, int y) const;
};
