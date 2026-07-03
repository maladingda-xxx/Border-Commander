#pragma once

#include "Core/Scene.h"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

#include <memory>

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
};
