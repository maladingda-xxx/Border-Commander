#include "Event/EventBus.h"

#include <algorithm>

namespace Event {

SubscriptionId EventBus::subscribe(EventType type, Callback callback) {
    SubscriptionId id = m_nextId++;
    m_subscriptions.push_back({id, type, std::move(callback)});
    return id;
}

void EventBus::unsubscribe(SubscriptionId id) {
    m_subscriptions.erase(
        std::remove_if(m_subscriptions.begin(), m_subscriptions.end(),
            [id](const Subscription& sub) { return sub.id == id; }),
        m_subscriptions.end());
}

void EventBus::emit(const Event& event) {
    for (const auto& sub : m_subscriptions) {
        if (sub.type == event.getType()) {
            sub.callback(event);
        }
    }
}

void EventBus::processQueue() {
    while (!m_queue.empty()) {
        auto event = std::move(m_queue.front());
        m_queue.pop();
        emit(*event);
    }
}

} // namespace Event
