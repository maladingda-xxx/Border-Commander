#pragma once

#include <SFML/Graphics/View.hpp>
#include <SFML/System/Vector2.hpp>

class Camera {
public:
    Camera();

    void move(float dx, float dy);
    void zoom(float factor);

    sf::Vector2f screenToWorld(const sf::Vector2i& screenPos, const sf::Vector2u& windowSize) const;
    sf::View getView(const sf::Vector2u& windowSize) const;

private:
    sf::Vector2f m_center;
    float m_zoom;
};
