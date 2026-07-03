#pragma once

#include "Core/Scene.h"

#include <memory>
#include <vector>

class SceneManager {
public:
    void pushScene(std::unique_ptr<Scene> scene);
    void popScene();
    void switchTo(std::unique_ptr<Scene> scene);
    void update(float dt);
    void render(sf::RenderWindow& window);
    void handleInput(const sf::Event& event);
    Scene* getCurrentScene();
    bool isEmpty() const;

private:
    std::vector<std::unique_ptr<Scene>> m_sceneStack;

    enum class Action { Push, Pop, Switch };
    struct PendingOp {
        Action action;
        std::unique_ptr<Scene> scene;
    };
    std::vector<PendingOp> m_pendingOps;

    void processPendingOperations();
};
