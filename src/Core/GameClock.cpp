#include "Core/GameClock.h"

#include <SFML/System/Time.hpp>

GameClock::GameClock()
    : m_deltaTime(0.0f)
    , m_fpsTimer(0.0f)
    , m_frameCount(0)
    , m_fps(0) {
}

float GameClock::restart() {
    m_deltaTime = m_clock.restart().asSeconds();

    m_fpsTimer += m_deltaTime;
    m_frameCount++;

    if (m_fpsTimer >= 1.0f) {
        m_fps = m_frameCount;
        m_frameCount = 0;
        m_fpsTimer -= 1.0f;
    }

    return m_deltaTime;
}

float GameClock::getDeltaTime() const {
    return m_deltaTime;
}

int GameClock::getFPS() const {
    return m_fps;
}
