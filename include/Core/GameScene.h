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

class Entity;
class GameClock;

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

    bool m_placementMode = false;
    BuildingType m_selectedBuildingType = BuildingType::Headquarters;

    void placeInitialHeadquarters();
    void handleClick(const sf::Vector2i& screenPos);
    void recalculateProduction();
    bool canPlaceBuilding(BuildingType type, const sf::Vector2i& tilePos);
    void placeBuilding(BuildingType type, const sf::Vector2i& tilePos);
    bool isTileOccupied(int x, int y) const;
};
