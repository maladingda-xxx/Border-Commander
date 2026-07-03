#include "AI/States.h"
#include "AI/StateMachine.h"
#include "Entity/Building.h"
#include "Entity/Entity.h"
#include "Entity/Unit.h"

#include <algorithm>
#include <cmath>

namespace {

bool isHostile(Faction a, Faction b) {
    return a != b;
}

} // anonymous namespace

// ─── IdleState ────────────────────────────────────────────────────────

void IdleState::enter(Unit* unit) {
    unit->stop();
}

void IdleState::execute(Unit* unit, float dt) {
    (void)dt;

    // Scan for enemies
    Entity* enemy = unit->findNearestHostile(DETECTION_RANGE);
    if (enemy) {
        auto combat = std::make_unique<CombatState>();
        unit->getStateMachine()->changeState(std::move(combat));
    }
}

void IdleState::exit(Unit* /*unit*/) {
}

// ─── MoveToState ──────────────────────────────────────────────────────

MoveToState::MoveToState(const sf::Vector2i& target)
    : m_target(target) {
}

void MoveToState::enter(Unit* unit) {
    unit->setTarget(m_target);
}

void MoveToState::execute(Unit* unit, float dt) {
    (void)dt;

    // Check for ambush
    Entity* enemy = unit->findNearestHostile(AMBUSH_RANGE);
    if (enemy) {
        auto combat = std::make_unique<CombatState>();
        unit->getStateMachine()->changeState(std::move(combat));
        return;
    }

    // Check if arrived
    if (!unit->isMoving()) {
        auto idle = std::make_unique<IdleState>();
        unit->getStateMachine()->changeState(std::move(idle));
    }
}

void MoveToState::exit(Unit* /*unit*/) {
}

// ─── CombatState ──────────────────────────────────────────────────────

CombatState::CombatState() {
}

void CombatState::enter(Unit* unit) {
    m_target = nullptr;
    m_attackTimer = 0.0f;
    unit->stop();
}

void CombatState::execute(Unit* unit, float dt) {
    // Validate or acquire target
    if (!m_target || !m_target->isActive()) {
        m_target = unit->findNearestHostile(CHASE_RANGE);
        if (!m_target) {
            auto idle = std::make_unique<IdleState>();
            unit->getStateMachine()->changeState(std::move(idle));
            return;
        }
        m_attackTimer = 0.0f;
    }

    sf::Vector2f targetPos = getEntityCenter(m_target);
    sf::Vector2f myPos = unit->getWorldPos();
    float dx = targetPos.x - myPos.x;
    float dy = targetPos.y - myPos.y;
    float dist = std::sqrt(dx * dx + dy * dy);

    float attackDist = ATTACK_RANGE * static_cast<float>(TILE_SIZE);

    if (dist > attackDist) {
        // Move toward target
        sf::Vector2i targetTile = unit->worldToTile(targetPos);
        unit->setTarget(targetTile);
        m_attackTimer = 0.0f;
    } else {
        // Attack
        unit->stop();
        m_attackTimer += dt;
        if (m_attackTimer >= ATTACK_INTERVAL) {
            m_attackTimer -= ATTACK_INTERVAL;
            int dmg = resolveDamage(unit, m_target);
            if (auto* targetUnit = dynamic_cast<Unit*>(m_target)) {
                targetUnit->takeDamage(dmg);
            } else if (auto* targetBuilding = dynamic_cast<Building*>(m_target)) {
                targetBuilding->takeDamage(dmg);
            }
        }
    }
}

void CombatState::exit(Unit* /*unit*/) {
    m_target = nullptr;
}

sf::Vector2f CombatState::getEntityCenter(const Entity* entity) const {
    if (auto* unit = dynamic_cast<const Unit*>(entity)) {
        return unit->getWorldPos();
    }
    // Building: center of tile position
    sf::Vector2i tilePos = entity->getTilePosition();
    return sf::Vector2f(
        static_cast<float>(tilePos.x * TILE_SIZE + TILE_SIZE / 2),
        static_cast<float>(tilePos.y * TILE_SIZE + TILE_SIZE / 2)
    );
}

int CombatState::resolveDamage(const Unit* attacker, const Entity* target) const {
    int defense = 0;
    if (auto* targetUnit = dynamic_cast<const Unit*>(target)) {
        defense = targetUnit->getDefense();
    }
    // Buildings have 0 defense
    return std::max(1, attacker->getAttack() - defense);
}
