#pragma once

#include <SFML/System/Clock.hpp>

class GameClock {
public:
    GameClock();

    float restart();
    float getDeltaTime() const;
    int getFPS() const;

private:
    sf::Clock m_clock;
    float m_deltaTime;
    float m_fpsTimer;
    int m_frameCount;
    int m_fps;
};
