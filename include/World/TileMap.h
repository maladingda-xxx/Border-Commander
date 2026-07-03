#pragma once

#include "World/Tile.h"

#include <SFML/System/Vector2.hpp>

#include <vector>

class TileMap {
public:
    static constexpr int WIDTH = 30;
    static constexpr int HEIGHT = 20;
    static constexpr int TILE_SIZE = 32;

    TileMap();

    const Tile& getTile(int x, int y) const;
    void setTile(int x, int y, const Tile& tile);

    bool isInBounds(int x, int y) const;
    std::vector<sf::Vector2i> getNeighbors(const sf::Vector2i& pos) const;

    sf::Vector2i worldToTile(const sf::Vector2f& worldPos) const;
    sf::Vector2f tileToWorld(const sf::Vector2i& tilePos) const;

    int getWidth() const { return WIDTH; }
    int getHeight() const { return HEIGHT; }
    int getTileSize() const { return TILE_SIZE; }

private:
    std::vector<Tile> m_tiles;
};
