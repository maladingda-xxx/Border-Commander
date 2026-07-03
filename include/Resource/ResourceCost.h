#pragma once

#include "ResourceType.h"

#include <unordered_map>

struct ResourceCost {
    std::unordered_map<ResourceType, int> costs;

    bool isEmpty() const { return costs.empty(); }

    int get(ResourceType type) const {
        auto it = costs.find(type);
        return (it != costs.end()) ? it->second : 0;
    }

    void set(ResourceType type, int amount) {
        costs[type] = amount;
    }
};
