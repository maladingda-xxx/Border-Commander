#include "World/Camera.h"

Camera::Camera()
    : m_center(0.0f, 0.0f)
    , m_zoom(1.0f) {
}

void Camera::move(float dx, float dy) {
    m_center.x += dx;
    m_center.y += dy;
}

void Camera::zoom(float factor) {
    m_zoom *= factor;
}

sf::Vector2f Camera::screenToWorld(const sf::Vector2i& screenPos, const sf::Vector2u& windowSize) const {
    float worldX = m_center.x + (static_cast<float>(screenPos.x) - static_cast<float>(windowSize.x) * 0.5f) / m_zoom;
    float worldY = m_center.y + (static_cast<float>(screenPos.y) - static_cast<float>(windowSize.y) * 0.5f) / m_zoom;
    return {worldX, worldY};
}

sf::View Camera::getView(const sf::Vector2u& windowSize) const {
    sf::Vector2f size{
        static_cast<float>(windowSize.x) / m_zoom,
        static_cast<float>(windowSize.y) / m_zoom
    };
    sf::View view(m_center, size);
    return view;
}
