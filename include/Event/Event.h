#pragma once

namespace Event {

enum class EventType {
    ResourceChanged,
    EntityDestroyed,
    BuildingPlaced,
    Combat,
    WaveStart,
    WaveEnd,
    GameOver,
    UICommand
};

class Event {
public:
    Event(EventType type) : m_type(type), m_timestamp(0.0f) {}
    virtual ~Event() = default;

    EventType getType() const { return m_type; }
    float getTimestamp() const { return m_timestamp; }
    void setTimestamp(float ts) { m_timestamp = ts; }

private:
    EventType m_type;
    float m_timestamp;
};

} // namespace Event
