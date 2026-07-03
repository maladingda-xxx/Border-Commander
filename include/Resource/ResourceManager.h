#pragma once

#include "ResourceType.h"
#include "ResourceCost.h"

namespace Event {
    class EventBus;
}

class ResourceManager {
public:
    ResourceManager();

    void setEventBus(Event::EventBus* bus) { m_eventBus = bus; }

    void addResource(ResourceType type, int amount);
    bool tryConsume(const ResourceCost& cost);
    bool canAfford(const ResourceCost& cost) const;

    int getCurrent(ResourceType type) const;
    int getMax(ResourceType type) const;
    void setMax(ResourceType type, int value);

    void setProduction(ResourceType type, float rate);
    float getProduction(ResourceType type) const;

    void update(float dt);

private:
    struct ResourcePool {
        int current = 0;
        int max = 0;
        float productionRate = 0.0f;
        float accumulator = 0.0f;
    };

    ResourcePool m_pools[3];
    Event::EventBus* m_eventBus = nullptr;

    void emitResourceChanged(ResourceType type, int oldValue, int newValue);
};
