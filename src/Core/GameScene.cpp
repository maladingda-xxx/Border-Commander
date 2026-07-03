#include "Core/GameScene.h"
#include "Core/GameClock.h"
#include "Entity/Building.h"
#include "Entity/Entity.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

#include <sstream>

GameScene::GameScene(const GameClock* clock)
    : m_clock(clock) {
    m_resourceManager.setEventBus(&m_eventBus);
}

void GameScene::onEnter() {
    if (!m_font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
        return;
    }

    m_fpsText = std::make_unique<sf::Text>(m_font, "", 16);
    m_fpsText->setFillColor(sf::Color::White);
    m_fpsText->setPosition({10.0f, 10.0f});

    m_resourceText = std::make_unique<sf::Text>(m_font, "", 16);
    m_resourceText->setFillColor(sf::Color::White);
    m_resourceText->setPosition({10.0f, 30.0f});

    m_statusText = std::make_unique<sf::Text>(m_font, "", 16);
    m_statusText->setFillColor(sf::Color(200, 200, 200));
    m_statusText->setPosition({10.0f, 50.0f});

    placeInitialHeadquarters();
}

void GameScene::onExit() {
    m_fpsText.reset();
    m_resourceText.reset();
    m_statusText.reset();
    m_entities.clear();
}

void GameScene::update(float dt) {
    if (!m_fpsText) {
        return;
    }

    std::ostringstream ss;
    ss << "FPS: " << m_clock->getFPS();
    m_fpsText->setString(ss.str());

    m_resourceManager.update(dt);

    for (auto& entity : m_entities) {
        entity->update(dt);
    }

    recalculateProduction();

    // Update status text
    if (m_placementMode) {
        Building tempBuilding(m_selectedBuildingType);
        auto cost = tempBuilding.getCost();
        std::ostringstream status;
        status << "Placing " << tempBuilding.getName()
               << " (Gold: " << cost.get(ResourceType::Gold)
               << " Food: " << cost.get(ResourceType::Food)
               << ") | Right-click to cancel";
        m_statusText->setString(status.str());
    } else {
        m_statusText->setString("1-4: Build | Drag: Pan");
    }
}

void GameScene::render(sf::RenderWindow& window) {
    m_windowSize = window.getSize();

    window.setView(m_camera.getView(window.getSize()));

    sf::RectangleShape tileRect({
        static_cast<float>(TileMap::TILE_SIZE),
        static_cast<float>(TileMap::TILE_SIZE)
    });

    for (int y = 0; y < TileMap::HEIGHT; ++y) {
        for (int x = 0; x < TileMap::WIDTH; ++x) {
            const Tile& tile = m_tileMap.getTile(x, y);
            sf::Vector2f pos = m_tileMap.tileToWorld({x, y});
            tileRect.setPosition(pos);

            switch (tile.terrainType) {
            case TerrainType::Grass:
                tileRect.setFillColor(sf::Color::Green);
                break;
            case TerrainType::Dirt:
                tileRect.setFillColor(sf::Color(139, 69, 19));
                break;
            case TerrainType::Water:
                tileRect.setFillColor(sf::Color::Blue);
                break;
            case TerrainType::Mountain:
                tileRect.setFillColor(sf::Color(128, 128, 128));
                break;
            }

            window.draw(tileRect);

            // Darken occupied tiles
            if (tile.occupied) {
                sf::RectangleShape overlay({
                    static_cast<float>(TileMap::TILE_SIZE),
                    static_cast<float>(TileMap::TILE_SIZE)
                });
                overlay.setPosition(pos);
                overlay.setFillColor(sf::Color(0, 0, 0, 60));
                window.draw(overlay);
            }
        }
    }

    // Draw entities
    for (auto& entity : m_entities) {
        if (entity->isActive()) {
            sf::Vector2f worldPos = m_tileMap.tileToWorld(entity->getTilePosition());
            entity->render(window, worldPos, TileMap::TILE_SIZE);
        }
    }

    // Draw placement preview
    if (m_placementMode) {
        sf::Vector2f worldPos = m_camera.screenToWorld(m_mouseScreenPos, m_windowSize);
        sf::Vector2i hoveredTile = m_tileMap.worldToTile(worldPos);

        Building preview(m_selectedBuildingType);
        auto size = preview.getSize();
        bool valid = canPlaceBuilding(m_selectedBuildingType, hoveredTile);

        sf::Color previewColor = preview.getColor();
        previewColor.a = 120;
        if (!valid) {
            previewColor = sf::Color(255, 0, 0, 100);
        }

        for (int dy = 0; dy < size.y; ++dy) {
            for (int dx = 0; dx < size.x; ++dx) {
                int tx = hoveredTile.x + dx;
                int ty = hoveredTile.y + dy;
                sf::Vector2f p = m_tileMap.tileToWorld({tx, ty});
                tileRect.setPosition(p);
                tileRect.setFillColor(previewColor);
                window.draw(tileRect);
            }
        }
    }

    window.setView(window.getDefaultView());

    // Resource HUD
    std::ostringstream resStr;
    resStr << "Gold: " << m_resourceManager.getCurrent(ResourceType::Gold)
           << "/" << m_resourceManager.getMax(ResourceType::Gold)
           << "  Food: " << m_resourceManager.getCurrent(ResourceType::Food)
           << "/" << m_resourceManager.getMax(ResourceType::Food)
           << "  Pop: " << m_resourceManager.getCurrent(ResourceType::Population)
           << "/" << m_resourceManager.getMax(ResourceType::Population);
    m_resourceText->setString(resStr.str());

    window.draw(*m_resourceText);
    window.draw(*m_statusText);
    window.draw(*m_fpsText);
}

void GameScene::handleInput(const sf::Event& event) {
    if (event.is<sf::Event::KeyPressed>()) {
        const auto& key = event.getIf<sf::Event::KeyPressed>();

        switch (key->code) {
        case sf::Keyboard::Key::Num1:
            m_placementMode = true;
            m_selectedBuildingType = BuildingType::Headquarters;
            break;
        case sf::Keyboard::Key::Num2:
            m_placementMode = true;
            m_selectedBuildingType = BuildingType::Barracks;
            break;
        case sf::Keyboard::Key::Num3:
            m_placementMode = true;
            m_selectedBuildingType = BuildingType::Farm;
            break;
        case sf::Keyboard::Key::Num4:
            m_placementMode = true;
            m_selectedBuildingType = BuildingType::GoldMine;
            break;
        case sf::Keyboard::Key::Escape:
            m_placementMode = false;
            break;
        default:
            break;
        }
    } else if (event.is<sf::Event::MouseButtonPressed>()) {
        const auto& mb = event.getIf<sf::Event::MouseButtonPressed>();
        if (mb->button == sf::Mouse::Button::Left) {
            m_dragging = true;
            m_lastMousePos = mb->position;
            m_mouseDownPos = mb->position;
        } else if (mb->button == sf::Mouse::Button::Right) {
            m_placementMode = false;
        }
    } else if (event.is<sf::Event::MouseButtonReleased>()) {
        const auto& mb = event.getIf<sf::Event::MouseButtonReleased>();
        if (mb->button == sf::Mouse::Button::Left) {
            m_dragging = false;
            int dx = mb->position.x - m_mouseDownPos.x;
            int dy = mb->position.y - m_mouseDownPos.y;
            if (std::abs(dx) < 3 && std::abs(dy) < 3) {
                handleClick(mb->position);
            }
        }
    } else if (event.is<sf::Event::MouseMoved>()) {
        const auto& mm = event.getIf<sf::Event::MouseMoved>();
        m_mouseScreenPos = mm->position;
        if (m_dragging) {
            float dx = static_cast<float>(mm->position.x - m_lastMousePos.x);
            float dy = static_cast<float>(mm->position.y - m_lastMousePos.y);
            m_camera.move(-dx, -dy);
            m_lastMousePos = mm->position;
        }
    }
}

void GameScene::placeInitialHeadquarters() {
    int hqX = 14;
    int hqY = 8;

    auto hq = std::make_unique<Building>(BuildingType::Headquarters);
    hq->setTilePosition({hqX, hqY});

    for (const auto& tile : hq->getOccupiedTiles()) {
        Tile t = m_tileMap.getTile(tile.x, tile.y);
        t.occupied = true;
        m_tileMap.setTile(tile.x, tile.y, t);
    }

    m_entities.push_back(std::move(hq));
}

void GameScene::handleClick(const sf::Vector2i& screenPos) {
    if (!m_placementMode) {
        return;
    }

    sf::Vector2f worldPos = m_camera.screenToWorld(screenPos, m_windowSize);
    sf::Vector2i tilePos = m_tileMap.worldToTile(worldPos);

    placeBuilding(m_selectedBuildingType, tilePos);
}

void GameScene::recalculateProduction() {
    float totalGold = 0.0f;
    float totalFood = 0.0f;

    for (auto& entity : m_entities) {
        auto* building = dynamic_cast<Building*>(entity.get());
        if (building) {
            totalGold += building->getProductionRate(ResourceType::Gold);
            totalFood += building->getProductionRate(ResourceType::Food);
        }
    }

    m_resourceManager.setProduction(ResourceType::Gold, totalGold);
    m_resourceManager.setProduction(ResourceType::Food, totalFood);
}

bool GameScene::canPlaceBuilding(BuildingType type, const sf::Vector2i& tilePos) {
    Building temp(type);
    temp.setTilePosition(tilePos);
    auto size = temp.getSize();
    auto cost = temp.getCost();

    if (!m_resourceManager.canAfford(cost)) {
        return false;
    }

    for (int dy = 0; dy < size.y; ++dy) {
        for (int dx = 0; dx < size.x; ++dx) {
            int tx = tilePos.x + dx;
            int ty = tilePos.y + dy;

            if (!m_tileMap.isInBounds(tx, ty)) {
                return false;
            }

            const Tile& tile = m_tileMap.getTile(tx, ty);
            if (!tile.isWalkable()) {
                return false;
            }

            if (tile.occupied) {
                return false;
            }
        }
    }

    return true;
}

void GameScene::placeBuilding(BuildingType type, const sf::Vector2i& tilePos) {
    if (!canPlaceBuilding(type, tilePos)) {
        return;
    }

    Building temp(type);
    temp.setTilePosition(tilePos);
    auto cost = temp.getCost();

    if (!m_resourceManager.tryConsume(cost)) {
        return;
    }

    auto building = std::make_unique<Building>(type);
    building->setTilePosition(tilePos);

    for (const auto& tile : building->getOccupiedTiles()) {
        Tile t = m_tileMap.getTile(tile.x, tile.y);
        t.occupied = true;
        m_tileMap.setTile(tile.x, tile.y, t);
    }

    m_entities.push_back(std::move(building));
}

bool GameScene::isTileOccupied(int x, int y) const {
    if (!m_tileMap.isInBounds(x, y)) {
        return true;
    }
    return m_tileMap.getTile(x, y).occupied;
}
