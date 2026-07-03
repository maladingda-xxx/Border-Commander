#include "Core/GameScene.h"
#include "Core/GameClock.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

#include <sstream>

GameScene::GameScene(const GameClock* clock)
    : m_clock(clock) {
}

void GameScene::onEnter() {
    if (!m_font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
        return;
    }

    m_fpsText = std::make_unique<sf::Text>(m_font, "", 18);
    m_fpsText->setFillColor(sf::Color::White);
    m_fpsText->setPosition({10.0f, 10.0f});
}

void GameScene::onExit() {
    m_fpsText.reset();
}

void GameScene::update(float /*dt*/) {
    if (!m_fpsText) {
        return;
    }
    std::ostringstream ss;
    ss << "FPS: " << m_clock->getFPS();
    m_fpsText->setString(ss.str());
}

void GameScene::render(sf::RenderWindow& window) {
    window.setView(m_camera.getView(window.getSize()));

    sf::RectangleShape tileRect({static_cast<float>(TileMap::TILE_SIZE), static_cast<float>(TileMap::TILE_SIZE)});

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
                tileRect.setFillColor(sf::Color(139, 69, 19)); // saddle brown
                break;
            case TerrainType::Water:
                tileRect.setFillColor(sf::Color::Blue);
                break;
            case TerrainType::Mountain:
                tileRect.setFillColor(sf::Color(128, 128, 128)); // gray
                break;
            }

            window.draw(tileRect);
        }
    }

    window.setView(window.getDefaultView());

    if (m_fpsText) {
        window.draw(*m_fpsText);
    }
}

void GameScene::handleInput(const sf::Event& event) {
    if (event.is<sf::Event::MouseButtonPressed>()) {
        const auto& mb = event.getIf<sf::Event::MouseButtonPressed>();
        if (mb->button == sf::Mouse::Button::Left) {
            m_dragging = true;
            m_lastMousePos = mb->position;
        }
    } else if (event.is<sf::Event::MouseButtonReleased>()) {
        const auto& mb = event.getIf<sf::Event::MouseButtonReleased>();
        if (mb->button == sf::Mouse::Button::Left) {
            m_dragging = false;
        }
    } else if (event.is<sf::Event::MouseMoved>()) {
        if (m_dragging) {
            const auto& mm = event.getIf<sf::Event::MouseMoved>();
            float dx = static_cast<float>(mm->position.x - m_lastMousePos.x);
            float dy = static_cast<float>(mm->position.y - m_lastMousePos.y);
            m_camera.move(-dx, -dy);
            m_lastMousePos = mm->position;
        }
    }
}
