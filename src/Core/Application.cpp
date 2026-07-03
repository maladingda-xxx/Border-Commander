#include "Core/Application.h"
#include "Core/Window.h"
#include "Core/Engine.h"
#include "Core/Game.h"

Application::Application() = default;
Application::~Application() = default;

bool Application::initialize() {
    m_window = std::make_unique<Window>();
    if (!m_window->create(1024, 768, "Border Commander")) {
        return false;
    }

    m_engine = std::make_unique<Engine>();
    if (!m_engine->initialize()) {
        return false;
    }

    m_game = std::make_unique<Game>();
    if (!m_game->initialize(m_window.get())) {
        return false;
    }

    return true;
}

void Application::run() {
    if (m_game) {
        m_game->run();
    }
}

void Application::shutdown() {
    if (m_game) {
        m_game->shutdown();
        m_game.reset();
    }
    if (m_engine) {
        m_engine->shutdown();
        m_engine.reset();
    }
    if (m_window) {
        m_window->close();
        m_window.reset();
    }
}
