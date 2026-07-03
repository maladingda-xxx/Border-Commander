#pragma once

#include "Event/EventBus.h"

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio/Music.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    void initialize(Event::EventBus* bus);
    void shutdown();

private:
    void loadOrGenerate(const std::string& name, const std::string& path, float freq, float duration);
    void playSound(const std::string& name);
    void generateBeep(const std::string& name, float freq, float duration);

    void onCombat(const Event::Event& e);
    void onEntityDestroyed(const Event::Event& e);
    void onBuildingPlaced(const Event::Event& e);
    void onWaveStart(const Event::Event& e);
    void onGameOver(const Event::Event& e);

    std::unordered_map<std::string, sf::SoundBuffer> m_buffers;
    std::vector<std::unique_ptr<sf::Sound>> m_activeSounds;
    sf::Music m_music;
    Event::EventBus* m_eventBus = nullptr;
    std::vector<Event::SubscriptionId> m_subscriptions;
};
