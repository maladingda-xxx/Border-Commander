#pragma once

#include <SFML/Graphics.hpp>

#include <optional>
#include <string>

class Window {
public:
    bool create(unsigned int width, unsigned int height, const std::string& title);
    void close();
    bool isOpen() const;
    std::optional<sf::Event> pollEvent();
    void clear(const sf::Color& color = sf::Color::Black);
    void display();
    sf::Vector2u getSize() const;
    sf::RenderWindow& getRenderTarget();

private:
    sf::RenderWindow m_window;
};
