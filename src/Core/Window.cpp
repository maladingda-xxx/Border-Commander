#include "Core/Window.h"

bool Window::create(unsigned int width, unsigned int height, const std::string& title) {
    m_window.create(sf::VideoMode({width, height}), title);
    return m_window.isOpen();
}

void Window::close() {
    m_window.close();
}

bool Window::isOpen() const {
    return m_window.isOpen();
}

std::optional<sf::Event> Window::pollEvent() {
    return m_window.pollEvent();
}

void Window::clear(const sf::Color& color) {
    m_window.clear(color);
}

void Window::display() {
    m_window.display();
}

sf::Vector2u Window::getSize() const {
    return m_window.getSize();
}

sf::RenderWindow& Window::getRenderTarget() {
    return m_window;
}
