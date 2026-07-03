#include "Resource/ResourceManager.h"

#include "Event/EventBus.h"
#include "Event/Events.h"

#include <algorithm>

static size_t toIndex(ResourceType type) {
    return static_cast<size_t>(type);
}

ResourceManager::ResourceManager() {
    m_pools[toIndex(ResourceType::Gold)].current = 300;
    m_pools[toIndex(ResourceType::Gold)].max = 500;

    m_pools[toIndex(ResourceType::Food)].current = 200;
    m_pools[toIndex(ResourceType::Food)].max = 300;

    m_pools[toIndex(ResourceType::Population)].current = 0;
    m_pools[toIndex(ResourceType::Population)].max = 10;
}

void ResourceManager::addResource(ResourceType type, int amount) {
    auto& pool = m_pools[toIndex(type)];
    int oldValue = pool.current;
    pool.current = std::min(pool.current + amount, pool.max);

    if (pool.current != oldValue) {
        emitResourceChanged(type, oldValue, pool.current);
    }
}

bool ResourceManager::tryConsume(const ResourceCost& cost) {
    if (!canAfford(cost)) {
        return false;
    }

    for (const auto& [type, amount] : cost.costs) {
        auto& pool = m_pools[toIndex(type)];
        int oldValue = pool.current;
        pool.current -= amount;
        emitResourceChanged(type, oldValue, pool.current);
    }

    return true;
}

bool ResourceManager::canAfford(const ResourceCost& cost) const {
    for (const auto& [type, amount] : cost.costs) {
        if (m_pools[toIndex(type)].current < amount) {
            return false;
        }
    }
    return true;
}

int ResourceManager::getCurrent(ResourceType type) const {
    return m_pools[toIndex(type)].current;
}

int ResourceManager::getMax(ResourceType type) const {
    return m_pools[toIndex(type)].max;
}

void ResourceManager::setMax(ResourceType type, int value) {
    auto& pool = m_pools[toIndex(type)];
    pool.max = value;
    if (pool.current > pool.max) {
        int oldValue = pool.current;
        pool.current = pool.max;
        emitResourceChanged(type, oldValue, pool.current);
    }
}

void ResourceManager::setProduction(ResourceType type, float rate) {
    m_pools[toIndex(type)].productionRate = rate;
}

float ResourceManager::getProduction(ResourceType type) const {
    return m_pools[toIndex(type)].productionRate;
}

void ResourceManager::update(float dt) {
    for (int i = 0; i < 3; ++i) {
        auto& pool = m_pools[i];
        if (pool.productionRate != 0.0f) {
            pool.accumulator += pool.productionRate * dt;

            int wholeUnits = static_cast<int>(pool.accumulator);
            if (wholeUnits != 0) {
                pool.accumulator -= static_cast<float>(wholeUnits);

                auto type = static_cast<ResourceType>(i);
                int oldValue = pool.current;
                pool.current = std::min(pool.current + wholeUnits, pool.max);

                if (pool.current != oldValue) {
                    emitResourceChanged(type, oldValue, pool.current);
                }
            }
        }
    }
}

void ResourceManager::emitResourceChanged(ResourceType type, int oldValue, int newValue) {
    if (m_eventBus) {
        m_eventBus->emit(Event::ResourceChangedEvent{type, oldValue, newValue});
    }
}
