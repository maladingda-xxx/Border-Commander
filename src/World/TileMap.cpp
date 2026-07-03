#include "World/TileMap.h"

#include <random>

TileMap::TileMap()
    : m_tiles(WIDTH * HEIGHT) {
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            int index = y * WIDTH + x;
            float r = dist(rng);

            if (r < 0.05f) {
                m_tiles[index].terrainType = TerrainType::Mountain;
            } else if (r < 0.12f) {
                m_tiles[index].terrainType = TerrainType::Water;
            } else if (r < 0.30f) {
                m_tiles[index].terrainType = TerrainType::Dirt;
            } else {
                m_tiles[index].terrainType = TerrainType::Grass;
            }
        }
    }
}

const Tile& TileMap::getTile(int x, int y) const {
    return m_tiles[y * WIDTH + x];
}

void TileMap::setTile(int x, int y, const Tile& tile) {
    m_tiles[y * WIDTH + x] = tile;
}

bool TileMap::isInBounds(int x, int y) const {
    return x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT;
}

std::vector<sf::Vector2i> TileMap::getNeighbors(const sf::Vector2i& pos) const {
    std::vector<sf::Vector2i> neighbors;
    const sf::Vector2i dirs[] = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}
    };

    for (const auto& dir : dirs) {
        int nx = pos.x + dir.x;
        int ny = pos.y + dir.y;
        if (isInBounds(nx, ny)) {
            neighbors.push_back({nx, ny});
        }
    }

    return neighbors;
}

sf::Vector2i TileMap::worldToTile(const sf::Vector2f& worldPos) const {
    return sf::Vector2i{
        static_cast<int>(worldPos.x / TILE_SIZE),
        static_cast<int>(worldPos.y / TILE_SIZE)
    };
}

sf::Vector2f TileMap::tileToWorld(const sf::Vector2i& tilePos) const {
    return sf::Vector2f{
        static_cast<float>(tilePos.x * TILE_SIZE),
        static_cast<float>(tilePos.y * TILE_SIZE)
    };
}
