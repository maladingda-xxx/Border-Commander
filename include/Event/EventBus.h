#pragma once

#include "Event.h"

#include <functional>
#include <vector>
#include <queue>
#include <memory>
#include <cstdint>

namespace Event {

using SubscriptionId = uint64_t;
using Callback = std::function<void(const Event&)>;

class EventBus {
public:
    EventBus() = default;

    SubscriptionId subscribe(EventType type, Callback callback);
    void unsubscribe(SubscriptionId id);

    void emit(const Event& event);

    template<typename T>
    void emitQueued(const T& event) {
        m_queue.push(std::make_unique<T>(event));
    }

    void processQueue();

private:
    struct Subscription {
        SubscriptionId id;
        EventType type;
        Callback callback;
    };

    std::vector<Subscription> m_subscriptions;
    std::queue<std::unique_ptr<Event>> m_queue;
    SubscriptionId m_nextId = 1;
};

} // namespace Event
