#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

class Scene {
public:
    virtual ~Scene() = default;

    virtual void onEnter() = 0;
    virtual void onExit() = 0;
    virtual void update(float dt) = 0;
    virtual void render(sf::RenderWindow& window) = 0;
    virtual void handleInput(const sf::Event& event) = 0;

    bool isActive() const { return m_active; }
    void setActive(bool active) { m_active = active; }

protected:
    bool m_active = false;
};
