#pragma once

#include <SFML/System/Vector2.hpp>

#include <vector>

class TileMap;

class Pathfinder {
public:
    explicit Pathfinder(const TileMap* tileMap);

    std::vector<sf::Vector2i> findPath(const sf::Vector2i& start, const sf::Vector2i& end) const;

private:
    const TileMap* m_tileMap;
};
