#pragma once

#include <SFML/System/Vector2.hpp>

enum class TerrainType {
    Grass,
    Dirt,
    Water,
    Mountain
};

struct Tile {
    TerrainType terrainType = TerrainType::Grass;
    bool occupied = false;

    bool isWalkable() const {
        return terrainType == TerrainType::Grass || terrainType == TerrainType::Dirt;
    }
};
