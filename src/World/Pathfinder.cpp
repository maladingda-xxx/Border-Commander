#include "World/Pathfinder.h"
#include "World/TileMap.h"

#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_map>

namespace {

int heuristic(const sf::Vector2i& a, const sf::Vector2i& b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

// Hash function for sf::Vector2i to use with unordered_map
struct VectorHash {
    std::size_t operator()(const sf::Vector2i& v) const {
        return static_cast<std::size_t>(v.x) ^ (static_cast<std::size_t>(v.y) << 16);
    }
};

struct Node {
    sf::Vector2i pos;
    int g;
    int f;

    bool operator>(const Node& other) const {
        return f > other.f;
    }
};

} // anonymous namespace

Pathfinder::Pathfinder(const TileMap* tileMap)
    : m_tileMap(tileMap) {
}

std::vector<sf::Vector2i> Pathfinder::findPath(const sf::Vector2i& start, const sf::Vector2i& end) const {
    if (!m_tileMap->isInBounds(start.x, start.y) || !m_tileMap->isInBounds(end.x, end.y)) {
        return {};
    }

    if (!m_tileMap->getTile(end.x, end.y).isWalkable()) {
        return {};
    }

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openSet;
    std::unordered_map<sf::Vector2i, sf::Vector2i, VectorHash> cameFrom;
    std::unordered_map<sf::Vector2i, int, VectorHash> gScore;

    openSet.push({start, 0, heuristic(start, end)});
    gScore[start] = 0;

    while (!openSet.empty()) {
        Node current = openSet.top();
        openSet.pop();

        if (current.pos == end) {
            std::vector<sf::Vector2i> path;
            sf::Vector2i step = end;
            while (step != start) {
                path.push_back(step);
                step = cameFrom[step];
            }
            path.push_back(start);
            std::reverse(path.begin(), path.end());
            return path;
        }

        auto neighbors = m_tileMap->getNeighbors(current.pos);
        for (const auto& neighbor : neighbors) {
            if (!m_tileMap->getTile(neighbor.x, neighbor.y).isWalkable()) {
                continue;
            }

            int tentativeG = current.g + 1;
            auto it = gScore.find(neighbor);
            if (it == gScore.end() || tentativeG < it->second) {
                cameFrom[neighbor] = current.pos;
                gScore[neighbor] = tentativeG;
                int f = tentativeG + heuristic(neighbor, end);
                openSet.push({neighbor, tentativeG, f});
            }
        }
    }

    return {};
}
