#include "Manager/AudioManager.h"
#include "Event/EventBus.h"
#include "Event/Events.h"

#include <SFML/Audio/SoundChannel.hpp>

#include <cmath>
#include <cstdint>
#include <vector>

AudioManager::AudioManager() = default;
AudioManager::~AudioManager() = default;

void AudioManager::initialize(Event::EventBus* bus) {
    m_eventBus = bus;
    if (!m_eventBus) return;

    // Subscribe to events
    auto id1 = m_eventBus->subscribe(Event::EventType::Combat,
        [this](const Event::Event& e) { onCombat(e); });
    m_subscriptions.push_back(id1);

    auto id2 = m_eventBus->subscribe(Event::EventType::EntityDestroyed,
        [this](const Event::Event& e) { onEntityDestroyed(e); });
    m_subscriptions.push_back(id2);

    auto id3 = m_eventBus->subscribe(Event::EventType::BuildingPlaced,
        [this](const Event::Event& e) { onBuildingPlaced(e); });
    m_subscriptions.push_back(id3);

    auto id4 = m_eventBus->subscribe(Event::EventType::WaveStart,
        [this](const Event::Event& e) { onWaveStart(e); });
    m_subscriptions.push_back(id4);

    auto id5 = m_eventBus->subscribe(Event::EventType::GameOver,
        [this](const Event::Event& e) { onGameOver(e); });
    m_subscriptions.push_back(id5);

    // Load sounds (or generate beeps as fallback)
    loadOrGenerate("hit", "assets/audio/hit.wav", 440.0f, 0.08f);
    loadOrGenerate("build", "assets/audio/build.wav", 660.0f, 0.15f);
    loadOrGenerate("kill", "assets/audio/kill.wav", 220.0f, 0.2f);
    loadOrGenerate("wave", "assets/audio/wave.wav", 880.0f, 0.3f);
    loadOrGenerate("gameover", "assets/audio/gameover.wav", 220.0f, 0.5f);

    // Background music (optional)
    [[maybe_unused]] bool musicLoaded = m_music.openFromFile("assets/audio/bgm.ogg");
    if (musicLoaded) {
        m_music.setLooping(true);
        m_music.setVolume(40.0f);
        m_music.play();
    }
}

void AudioManager::shutdown() {
    if (m_eventBus) {
        for (auto id : m_subscriptions) {
            m_eventBus->unsubscribe(id);
        }
        m_subscriptions.clear();
    }
    m_music.stop();
    m_activeSounds.clear();
    m_buffers.clear();
    m_eventBus = nullptr;
}

void AudioManager::loadOrGenerate(const std::string& name, const std::string& path,
                                   float freq, float duration) {
    sf::SoundBuffer buffer;
    [[maybe_unused]] bool loaded = buffer.loadFromFile(path);
    if (loaded) {
        m_buffers[name] = std::move(buffer);
    } else {
        generateBeep(name, freq, duration);
    }
}

void AudioManager::playSound(const std::string& name) {
    auto it = m_buffers.find(name);
    if (it == m_buffers.end()) return;

    // Clean up stopped sounds
    m_activeSounds.erase(
        std::remove_if(m_activeSounds.begin(), m_activeSounds.end(),
            [](const std::unique_ptr<sf::Sound>& s) {
                return s->getStatus() == sf::Sound::Status::Stopped;
            }),
        m_activeSounds.end());

    auto sound = std::make_unique<sf::Sound>(it->second);
    sound->setVolume(30.0f);
    sound->play();
    m_activeSounds.push_back(std::move(sound));
}

void AudioManager::generateBeep(const std::string& name, float freq, float duration) {
    unsigned sampleRate = 44100;
    unsigned sampleCount = static_cast<unsigned>(sampleRate * duration);
    std::vector<int16_t> samples(sampleCount);

    for (unsigned i = 0; i < sampleCount; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(sampleRate);
        float envelope = 1.0f - (t / duration);
        samples[i] = static_cast<int16_t>(
            std::sin(2.0f * 3.14159f * freq * t) * envelope * 8000.0f);
    }

    sf::SoundBuffer buffer;
    [[maybe_unused]] bool ok = buffer.loadFromSamples(samples.data(), sampleCount, 1, sampleRate,
        std::vector<sf::SoundChannel>{});
    m_buffers[name] = std::move(buffer);
}

void AudioManager::onCombat(const Event::Event& /*e*/) {
    playSound("hit");
}

void AudioManager::onEntityDestroyed(const Event::Event& /*e*/) {
    playSound("kill");
}

void AudioManager::onBuildingPlaced(const Event::Event& /*e*/) {
    playSound("build");
}

void AudioManager::onWaveStart(const Event::Event& /*e*/) {
    playSound("wave");
}

void AudioManager::onGameOver(const Event::Event& /*e*/) {
    m_music.stop();
    playSound("gameover");
}
