#include "Core/SceneManager.h"

void SceneManager::pushScene(std::unique_ptr<Scene> scene) {
    m_pendingOps.push_back({Action::Push, std::move(scene)});
}

void SceneManager::popScene() {
    m_pendingOps.push_back({Action::Pop, nullptr});
}

void SceneManager::switchTo(std::unique_ptr<Scene> scene) {
    m_pendingOps.push_back({Action::Switch, std::move(scene)});
}

void SceneManager::update(float dt) {
    processPendingOperations();
    if (!m_sceneStack.empty()) {
        m_sceneStack.back()->update(dt);
    }
}

void SceneManager::render(sf::RenderWindow& window) {
    processPendingOperations();
    for (auto& scene : m_sceneStack) {
        if (scene->isActive()) {
            scene->render(window);
        }
    }
}

void SceneManager::handleInput(const sf::Event& event) {
    processPendingOperations();
    if (!m_sceneStack.empty()) {
        m_sceneStack.back()->handleInput(event);
    }
}

Scene* SceneManager::getCurrentScene() {
    if (m_sceneStack.empty()) {
        return nullptr;
    }
    return m_sceneStack.back().get();
}

bool SceneManager::isEmpty() const {
    return m_sceneStack.empty();
}

void SceneManager::processPendingOperations() {
    for (auto& op : m_pendingOps) {
        switch (op.action) {
        case Action::Push:
            if (!m_sceneStack.empty()) {
                m_sceneStack.back()->onExit();
            }
            m_sceneStack.push_back(std::move(op.scene));
            m_sceneStack.back()->setActive(true);
            m_sceneStack.back()->onEnter();
            break;
        case Action::Pop:
            if (!m_sceneStack.empty()) {
                m_sceneStack.back()->onExit();
                m_sceneStack.back()->setActive(false);
                m_sceneStack.pop_back();
            }
            if (!m_sceneStack.empty()) {
                m_sceneStack.back()->onEnter();
            }
            break;
        case Action::Switch:
            while (!m_sceneStack.empty()) {
                m_sceneStack.back()->onExit();
                m_sceneStack.back()->setActive(false);
                m_sceneStack.pop_back();
            }
            m_sceneStack.push_back(std::move(op.scene));
            m_sceneStack.back()->setActive(true);
            m_sceneStack.back()->onEnter();
            break;
        }
    }
    m_pendingOps.clear();
}
