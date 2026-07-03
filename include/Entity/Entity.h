#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>

class Entity {
public:
    Entity();
    virtual ~Entity() = default;

    int getId() const { return m_id; }
    bool isActive() const { return m_active; }
    void setActive(bool active) { m_active = active; }

    sf::Vector2i getTilePosition() const { return m_tilePos; }
    void setTilePosition(const sf::Vector2i& pos) { m_tilePos = pos; }

    virtual void update(float /*dt*/) {}
    virtual void render(sf::RenderWindow& window, const sf::Vector2f& worldPos, int tileSize);

private:
    int m_id;
    bool m_active = true;
    sf::Vector2i m_tilePos;
    static int s_nextId;
};
