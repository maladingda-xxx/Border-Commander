#include "Core/GameScene.h"
#include "Core/GameClock.h"
#include "AI/States.h"
#include "Entity/Building.h"
#include "Entity/Entity.h"
#include "Entity/Unit.h"
#include "UI/BuildMenu.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

#include <random>
#include <sstream>

GameScene::GameScene(const GameClock* clock)
    : m_clock(clock)
    , m_buildMenu(m_font, &m_resourceManager) {
    bool fontLoaded = m_font.openFromFile("C:/Windows/Fonts/arial.ttf");
    (void)fontLoaded;
    m_resourceManager.setEventBus(&m_eventBus);
}

void GameScene::onEnter() {

    m_fpsText = std::make_unique<sf::Text>(m_font, "", 16);
    m_fpsText->setFillColor(sf::Color::White);
    m_fpsText->setPosition({10.0f, 10.0f});

    m_resourceText = std::make_unique<sf::Text>(m_font, "", 16);
    m_resourceText->setFillColor(sf::Color::White);
    m_resourceText->setPosition({10.0f, 30.0f});

    m_statusText = std::make_unique<sf::Text>(m_font, "", 16);
    m_statusText->setFillColor(sf::Color(200, 200, 200));
    m_statusText->setPosition({10.0f, 50.0f});

    m_gameOverText = std::make_unique<sf::Text>(m_font, "", 48);
    m_gameOverText->setFillColor(sf::Color::Red);
    m_gameOverText->setStyle(sf::Text::Bold);

    m_resourceManager.setMax(ResourceType::Population, 20);

    placeInitialHeadquarters();
}

void GameScene::onExit() {
    m_fpsText.reset();
    m_resourceText.reset();
    m_statusText.reset();
    m_gameOverText.reset();
    m_entities.clear();
}

void GameScene::update(float dt) {
    if (!m_fpsText) {
        return;
    }

    if (m_gameOver) {
        m_statusText->setString("GAME OVER - HQ destroyed");
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

    // Clean up dead entities
    m_entities.erase(
        std::remove_if(m_entities.begin(), m_entities.end(),
            [this](const std::unique_ptr<Entity>& e) {
                bool dead = false;
                if (auto* unit = dynamic_cast<Unit*>(e.get())) {
                    dead = !unit->isAlive();
                    if (dead) {
                        if (auto* enemy = dynamic_cast<Enemy*>(e.get())) {
                            m_resourceManager.addResource(ResourceType::Gold, enemy->getBounty());
                            ++m_killCount;
                        } else {
                            m_resourceManager.addResource(ResourceType::Population, -1);
                        }
                    }
                } else if (auto* building = dynamic_cast<Building*>(e.get())) {
                    dead = !building->isAlive();
                    if (dead) {
                        for (const auto& tile : building->getOccupiedTiles()) {
                            Tile t = m_tileMap.getTile(tile.x, tile.y);
                            t.occupied = false;
                            m_tileMap.setTile(tile.x, tile.y, t);
                        }
                        if (building->getBuildingType() == BuildingType::Headquarters) {
                            m_gameOver = true;
                        }
                    }
                }
                if (dead && e->getId() == m_selectedEntityId) {
                    m_selectedEntityId = -1;
                }
                return dead;
            }),
        m_entities.end());

    // Wave spawning
    if (!m_gameOver) {
        m_spawnManager.update(dt);

        if (m_spawnManager.shouldSpawn()) {
            spawnEnemy();
            m_spawnManager.confirmSpawn();
        }

        // Check if wave is cleared
        if (m_spawnManager.getState() == WaveState::AwaitingClear) {
            bool anyEnemyAlive = false;
            for (auto& e : m_entities) {
                if (auto* enemy = dynamic_cast<Enemy*>(e.get())) {
                    if (enemy->isActive() && enemy->isAlive()) {
                        anyEnemyAlive = true;
                        break;
                    }
                }
            }
            if (!anyEnemyAlive) {
                m_spawnManager.notifyWaveCleared();
            }
        }
    }

    m_buildMenu.update();

    // Status text
    if (m_buildMenu.isPlacementMode()) {
        Building tempBuilding(m_buildMenu.getSelectedType());
        auto cost = tempBuilding.getCost();
        std::ostringstream status;
        status << "Placing " << tempBuilding.getName()
               << " (Gold: " << cost.get(ResourceType::Gold)
               << " Food: " << cost.get(ResourceType::Food)
               << ") | Right-click to cancel";
        m_statusText->setString(status.str());
    } else if (m_selectedEntityId >= 0) {
        m_statusText->setString("Unit selected | Right-click to move | 1-4: Build | Drag: Pan");
    } else if (m_spawnManager.getCurrentWave() == 0) {
        std::ostringstream status;
        status << "Prepare! First wave in " << static_cast<int>(m_spawnManager.getRestTimeRemaining()) << "s";
        m_statusText->setString(status.str());
    } else {
        std::ostringstream status;
        auto state = m_spawnManager.getState();
        int wave = m_spawnManager.getCurrentWave();
        if (state == WaveState::Resting) {
            status << "Wave " << wave << " cleared! Next in "
                   << static_cast<int>(m_spawnManager.getRestTimeRemaining()) << "s";
        } else if (state == WaveState::Spawning) {
            status << "Wave " << wave << ": "
                   << m_spawnManager.getSpawnedThisWave() << "/"
                   << m_spawnManager.getTotalEnemiesThisWave() << " spawned";
        } else {
            status << "Wave " << wave << ": clear remaining enemies";
        }
        m_statusText->setString(status.str());
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
        if (!entity->isActive()) {
            continue;
        }

        // Units render at their smooth world position; buildings render at tile-aligned position
        sf::Vector2f worldPos;
        if (auto* unit = dynamic_cast<Unit*>(entity.get())) {
            worldPos = unit->getWorldPos();
        } else {
            worldPos = m_tileMap.tileToWorld(entity->getTilePosition());
        }
        entity->render(window, worldPos, TileMap::TILE_SIZE);

        // Selection highlight
        if (entity->getId() == m_selectedEntityId) {
            float size = static_cast<float>(TileMap::TILE_SIZE);
            sf::Vector2f drawPos = worldPos;
            // Re-center for units (which render centered on worldPos)
            if (dynamic_cast<Unit*>(entity.get())) {
                drawPos = {worldPos.x - size / 2.0f, worldPos.y - size / 2.0f};
            }

            sf::RectangleShape outline({size, size});
            outline.setPosition(drawPos);
            outline.setFillColor(sf::Color(0, 0, 0, 0));
            outline.setOutlineColor(sf::Color::White);
            outline.setOutlineThickness(2.0f);
            window.draw(outline);
        }
    }

    // Target crosshair for selected unit
    if (m_selectedEntityId >= 0) {
        for (auto& entity : m_entities) {
            if (entity->getId() == m_selectedEntityId) {
                if (auto* unit = dynamic_cast<Unit*>(entity.get())) {
                    if (unit->isMoving()) {
                        sf::Vector2f targetPos = m_tileMap.tileToWorld(unit->getTargetTile());
                        float ts = static_cast<float>(TileMap::TILE_SIZE);

                        // Horizontal line
                        sf::RectangleShape hLine({ts * 0.6f, 2.0f});
                        hLine.setPosition({targetPos.x + ts * 0.2f, targetPos.y + ts / 2.0f});
                        hLine.setFillColor(sf::Color(255, 255, 255, 180));
                        window.draw(hLine);

                        // Vertical line
                        sf::RectangleShape vLine({2.0f, ts * 0.6f});
                        vLine.setPosition({targetPos.x + ts / 2.0f, targetPos.y + ts * 0.2f});
                        vLine.setFillColor(sf::Color(255, 255, 255, 180));
                        window.draw(vLine);
                    }
                }
                break;
            }
        }
    }

    // Placement preview
    if (m_buildMenu.isPlacementMode()) {
        sf::Vector2f worldPos = m_camera.screenToWorld(m_mouseScreenPos, m_windowSize);
        sf::Vector2i hoveredTile = m_tileMap.worldToTile(worldPos);

        Building preview(m_buildMenu.getSelectedType());
        auto size = preview.getSize();
        bool valid = canPlaceBuilding(m_buildMenu.getSelectedType(), hoveredTile);

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
    resStr << "Wave: " << m_spawnManager.getCurrentWave() << "  Kills: " << m_killCount
           << "  |  Gold: " << m_resourceManager.getCurrent(ResourceType::Gold)
           << "/" << m_resourceManager.getMax(ResourceType::Gold)
           << "  Food: " << m_resourceManager.getCurrent(ResourceType::Food)
           << "/" << m_resourceManager.getMax(ResourceType::Food)
           << "  Pop: " << m_resourceManager.getCurrent(ResourceType::Population)
           << "/" << m_resourceManager.getMax(ResourceType::Population);
    m_resourceText->setString(resStr.str());

    window.draw(*m_resourceText);
    window.draw(*m_statusText);
    window.draw(*m_fpsText);

    // Build menu
    if (!m_gameOver) {
        m_buildMenu.render(window);
    }

    // Game over overlay
    if (m_gameOver) {
        sf::Vector2u winSize = window.getSize();
        m_gameOverText->setString("GAME OVER");
        sf::FloatRect bounds = m_gameOverText->getGlobalBounds();
        m_gameOverText->setPosition({
            (static_cast<float>(winSize.x) - bounds.size.x) / 2.0f,
            (static_cast<float>(winSize.y) - bounds.size.y) / 2.0f
        });
        window.draw(*m_gameOverText);
    }
}

void GameScene::handleInput(const sf::Event& event) {
    if (event.is<sf::Event::KeyPressed>()) {
        const auto& key = event.getIf<sf::Event::KeyPressed>();

        switch (key->code) {
        case sf::Keyboard::Key::Num1:
            m_buildMenu.selectByKey(BuildingType::Headquarters);
            m_selectedEntityId = -1;
            break;
        case sf::Keyboard::Key::Num2:
            m_buildMenu.selectByKey(BuildingType::Barracks);
            m_selectedEntityId = -1;
            break;
        case sf::Keyboard::Key::Num3:
            m_buildMenu.selectByKey(BuildingType::Farm);
            m_selectedEntityId = -1;
            break;
        case sf::Keyboard::Key::Num4:
            m_buildMenu.selectByKey(BuildingType::GoldMine);
            m_selectedEntityId = -1;
            break;
        case sf::Keyboard::Key::Escape:
            m_buildMenu.cancelPlacement();
            m_selectedEntityId = -1;
            break;
        default:
            break;
        }
    } else if (event.is<sf::Event::MouseButtonPressed>()) {
        const auto& mb = event.getIf<sf::Event::MouseButtonPressed>();
        if (mb->button == sf::Mouse::Button::Left) {
            // Check BuildMenu first
            if (m_buildMenu.handleClick(mb->position)) {
                // Handled by menu
            } else {
                m_dragging = true;
                m_lastMousePos = mb->position;
                m_mouseDownPos = mb->position;
            }
        } else if (mb->button == sf::Mouse::Button::Right) {
            m_buildMenu.cancelPlacement();
            handleRightClick(mb->position);
        }
    } else if (event.is<sf::Event::MouseButtonReleased>()) {
        const auto& mb = event.getIf<sf::Event::MouseButtonReleased>();
        if (mb->button == sf::Mouse::Button::Left) {
            m_dragging = false;
            int dx = mb->position.x - m_mouseDownPos.x;
            int dy = mb->position.y - m_mouseDownPos.y;
            if (std::abs(dx) < 3 && std::abs(dy) < 3) {
                if (m_buildMenu.isPlacementMode()) {
                    handleClick(mb->position);
                } else {
                    handleEntityClick(mb->position);
                }
            }
        }
    } else if (event.is<sf::Event::MouseMoved>()) {
        const auto& mm = event.getIf<sf::Event::MouseMoved>();
        m_mouseScreenPos = mm->position;
        m_buildMenu.handleMouseMove(mm->position);
        if (m_dragging) {
            float dx = static_cast<float>(mm->position.x - m_lastMousePos.x);
            float dy = static_cast<float>(mm->position.y - m_lastMousePos.y);
            m_camera.move(-dx, -dy);
            m_lastMousePos = mm->position;
        }
    }
}

// ─── Building placement ──────────────────────────────────────────────

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
    if (!m_buildMenu.isPlacementMode()) {
        return;
    }

    sf::Vector2f worldPos = m_camera.screenToWorld(screenPos, m_windowSize);
    sf::Vector2i tilePos = m_tileMap.worldToTile(worldPos);

    placeBuilding(m_buildMenu.getSelectedType(), tilePos);
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

// ─── Entity interaction ──────────────────────────────────────────────

void GameScene::handleEntityClick(const sf::Vector2i& screenPos) {
    sf::Vector2f worldPos = m_camera.screenToWorld(screenPos, m_windowSize);
    sf::Vector2i tilePos = m_tileMap.worldToTile(worldPos);

    Entity* clicked = findEntityAt(tilePos);
    if (!clicked) {
        m_selectedEntityId = -1;
        return;
    }

    // Check for barracks recruitment
    if (auto* building = dynamic_cast<Building*>(clicked)) {
        if (building->getBuildingType() == BuildingType::Barracks && building->isBuilt()) {
            tryRecruitSoldier(building);
            return;
        }
    }

    // Check for player unit selection
    if (auto* unit = dynamic_cast<Unit*>(clicked)) {
        if (unit->getFaction() == Faction::Player) {
            m_selectedEntityId = unit->getId();
            return;
        }
    }

    m_selectedEntityId = -1;
}

void GameScene::handleRightClick(const sf::Vector2i& screenPos) {
    if (m_buildMenu.isPlacementMode()) {
        m_buildMenu.cancelPlacement();
        return;
    }

    if (m_selectedEntityId < 0) {
        return;
    }

    sf::Vector2f worldPos = m_camera.screenToWorld(screenPos, m_windowSize);
    sf::Vector2i tilePos = m_tileMap.worldToTile(worldPos);

    commandMove(tilePos);
}

// ─── Unit management ─────────────────────────────────────────────────

void GameScene::tryRecruitSoldier(Building* barracks) {
    ResourceCost cost = Soldier::getRecruitCost();

    if (!m_resourceManager.canAfford(cost)) {
        return;
    }

    if (m_resourceManager.getCurrent(ResourceType::Population) >=
        m_resourceManager.getMax(ResourceType::Population)) {
        return;
    }

    sf::Vector2i spawnTile = findAdjacentEmptyTile(barracks->getTilePosition());
    if (spawnTile.x < 0) {
        return;
    }

    m_resourceManager.tryConsume(cost);
    m_resourceManager.addResource(ResourceType::Population, 1);

    auto soldier = std::make_unique<Soldier>();
    soldier->setTilePosition(spawnTile);
    soldier->setWorldPos(m_tileMap.tileToWorld(spawnTile) +
        sf::Vector2f(TileMap::TILE_SIZE / 2.0f, TileMap::TILE_SIZE / 2.0f));
    soldier->setEntityList(&m_entities);
    soldier->setEventBus(&m_eventBus);

    m_entities.push_back(std::move(soldier));
}

void GameScene::commandMove(const sf::Vector2i& tilePos) {
    if (!m_tileMap.isInBounds(tilePos.x, tilePos.y)) {
        return;
    }

    for (auto& entity : m_entities) {
        if (entity->getId() == m_selectedEntityId) {
            if (auto* unit = dynamic_cast<Unit*>(entity.get())) {
                unit->commandMoveTo(tilePos);
            }
            break;
        }
    }
}

void GameScene::spawnEnemy() {
    std::vector<sf::Vector2i> edgeTiles;
    for (int x = 0; x < TileMap::WIDTH; ++x) {
        if (isValidSpawnTile(x, 0))                     edgeTiles.push_back({x, 0});
        if (isValidSpawnTile(x, TileMap::HEIGHT - 1))   edgeTiles.push_back({x, TileMap::HEIGHT - 1});
    }
    for (int y = 1; y < TileMap::HEIGHT - 1; ++y) {
        if (isValidSpawnTile(0, y))                     edgeTiles.push_back({0, y});
        if (isValidSpawnTile(TileMap::WIDTH - 1, y))    edgeTiles.push_back({TileMap::WIDTH - 1, y});
    }

    if (edgeTiles.empty()) {
        return;
    }

    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, edgeTiles.size() - 1);
    auto spawnPos = edgeTiles[dist(rng)];

    int wave = m_spawnManager.getCurrentWave();

    sf::Vector2i hqTarget(14, 8);
    auto enemy = std::make_unique<Enemy>(wave, hqTarget,
        m_spawnManager.getHPBonus(), m_spawnManager.getAttackBonus());
    enemy->setTilePosition(spawnPos);
    enemy->setWorldPos(m_tileMap.tileToWorld(spawnPos) +
        sf::Vector2f(TileMap::TILE_SIZE / 2.0f, TileMap::TILE_SIZE / 2.0f));
    enemy->setEntityList(&m_entities);
    enemy->setEventBus(&m_eventBus);

    m_entities.push_back(std::move(enemy));
}

Entity* GameScene::findEntityAt(const sf::Vector2i& tilePos) {
    for (auto& entity : m_entities) {
        if (!entity->isActive()) {
            continue;
        }
        if (auto* building = dynamic_cast<Building*>(entity.get())) {
            for (const auto& t : building->getOccupiedTiles()) {
                if (t == tilePos) {
                    return entity.get();
                }
            }
        } else if (entity->getTilePosition() == tilePos) {
            return entity.get();
        }
    }
    return nullptr;
}

sf::Vector2i GameScene::findAdjacentEmptyTile(const sf::Vector2i& tilePos) {
    const sf::Vector2i dirs[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {-1, 1}, {1, -1}, {-1, -1}};
    for (const auto& dir : dirs) {
        sf::Vector2i adj(tilePos.x + dir.x, tilePos.y + dir.y);
        if (m_tileMap.isInBounds(adj.x, adj.y)) {
            const Tile& tile = m_tileMap.getTile(adj.x, adj.y);
            if (tile.isWalkable() && !tile.occupied) {
                return adj;
            }
        }
    }
    return {-1, -1};
}

bool GameScene::isValidSpawnTile(int x, int y) const {
    if (!m_tileMap.isInBounds(x, y)) {
        return false;
    }
    const Tile& tile = m_tileMap.getTile(x, y);
    return tile.isWalkable() && !tile.occupied;
}
