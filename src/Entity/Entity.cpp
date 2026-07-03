#include "Entity/Entity.h"

int Entity::s_nextId = 1;

Entity::Entity()
    : m_id(s_nextId++)
    , m_tilePos(0, 0) {
}

void Entity::render(sf::RenderWindow& /*window*/, const sf::Vector2f& /*worldPos*/, int /*tileSize*/) {
    // Base implementation: no rendering
}
