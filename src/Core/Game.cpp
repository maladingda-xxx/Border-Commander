#include "Core/Game.h"
#include "Core/Window.h"
#include "Core/SceneManager.h"
#include "Core/GameScene.h"

Game::Game() = default;
Game::~Game() = default;

bool Game::initialize(Window* window) {
    m_window = window;
    m_running = true;

    m_sceneManager = std::make_unique<SceneManager>();
    m_sceneManager->pushScene(std::make_unique<GameScene>(&m_clock));

    return true;
}

void Game::run() {
    float accumulator = 0.0f;

    while (m_running && m_window->isOpen()) {
        processEvents();

        float frameTime = m_clock.restart();
        accumulator += frameTime;

        while (accumulator >= FIXED_TIMESTEP) {
            fixedUpdate(FIXED_TIMESTEP);
            accumulator -= FIXED_TIMESTEP;
        }

        float alpha = accumulator / FIXED_TIMESTEP;
        render(alpha);
    }
}

void Game::shutdown() {
    m_running = false;
}

void Game::processEvents() {
    while (auto event = m_window->pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            m_running = false;
        }
        m_sceneManager->handleInput(*event);
    }
}

void Game::fixedUpdate(float dt) {
    m_sceneManager->update(dt);
}

void Game::render(float /*alpha*/) {
    m_window->clear();
    m_sceneManager->render(m_window->getRenderTarget());
    m_window->display();
}
