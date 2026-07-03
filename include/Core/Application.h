#pragma once

#include <memory>

class Window;
class Engine;
class Game;

class Application {
public:
    Application();
    ~Application();

    bool initialize();
    void run();
    void shutdown();

private:
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Engine> m_engine;
    std::unique_ptr<Game> m_game;
};
