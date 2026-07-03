#include "Core/GameScene.h"
#include "Core/GameClock.h"

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
    if (!m_fpsText) {
        return;
    }
    window.draw(*m_fpsText);
}

void GameScene::handleInput(const sf::Event& /*event*/) {
}
