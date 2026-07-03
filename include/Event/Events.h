#pragma once

#include "Event.h"
#include "Resource/ResourceType.h"

namespace Event {

struct ResourceChangedEvent : public Event {
    ResourceType resourceType;
    int oldValue;
    int newValue;

    ResourceChangedEvent(ResourceType rt, int ov, int nv)
        : Event(EventType::ResourceChanged)
        , resourceType(rt)
        , oldValue(ov)
        , newValue(nv) {}
};

struct EntityDestroyedEvent : public Event {
    int entityId;

    explicit EntityDestroyedEvent(int id)
        : Event(EventType::EntityDestroyed)
        , entityId(id) {}
};

struct BuildingPlacedEvent : public Event {
    int buildingType;
    int tileX;
    int tileY;

    BuildingPlacedEvent(int bt, int x, int y)
        : Event(EventType::BuildingPlaced)
        , buildingType(bt)
        , tileX(x)
        , tileY(y) {}
};

struct CombatEvent : public Event {
    int attackerId;
    int targetId;
    int damage;

    CombatEvent(int aid, int tid, int dmg)
        : Event(EventType::Combat)
        , attackerId(aid)
        , targetId(tid)
        , damage(dmg) {}
};

struct WaveStartEvent : public Event {
    int waveNumber;

    explicit WaveStartEvent(int wave)
        : Event(EventType::WaveStart)
        , waveNumber(wave) {}
};

struct GameOverEvent : public Event {
    bool isVictory;

    explicit GameOverEvent(bool victory)
        : Event(EventType::GameOver)
        , isVictory(victory) {}
};

} // namespace Event
