#pragma once

#include "Core/GameClock.h"

#include <memory>

class Window;
class SceneManager;

class Game {
public:
    Game();
    ~Game();

    bool initialize(Window* window);
    void run();
    void shutdown();

private:
    void processEvents();
    void fixedUpdate(float dt);
    void render(float alpha);

    Window* m_window;
    GameClock m_clock;
    std::unique_ptr<SceneManager> m_sceneManager;
    bool m_running;

    static constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;
};
