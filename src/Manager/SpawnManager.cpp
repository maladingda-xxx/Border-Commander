#include "Manager/SpawnManager.h"

#include <algorithm>

SpawnManager::SpawnManager() {
    m_timer = 10.0f; // initial prep time before wave 1
    m_restDuration = 10.0f;
}

void SpawnManager::update(float dt) {
    switch (m_state) {
    case WaveState::Resting:
        m_timer -= dt;
        if (m_timer <= 0.0f) {
            startWave(m_wave + 1);
        }
        break;

    case WaveState::Spawning:
        m_timer -= dt;
        break;

    case WaveState::AwaitingClear:
        // Waiting for all enemies to be killed
        break;
    }
}

bool SpawnManager::shouldSpawn() const {
    return m_state == WaveState::Spawning
        && m_spawnedCount < m_totalEnemies
        && m_timer <= 0.0f;
}

void SpawnManager::confirmSpawn() {
    ++m_spawnedCount;
    if (m_spawnedCount >= m_totalEnemies) {
        m_state = WaveState::AwaitingClear;
    } else {
        m_timer = m_spawnInterval;
    }
}

void SpawnManager::notifyWaveCleared() {
    startRest();
}

int SpawnManager::getHPBonus() const {
    return m_hpBonus;
}

int SpawnManager::getAttackBonus() const {
    return m_attackBonus;
}

void SpawnManager::startWave(int wave) {
    m_wave = wave;
    m_spawnedCount = 0;
    m_timer = 0.0f; // first spawn immediately
    m_state = WaveState::Spawning;

    // Wave config
    if (wave == 1) {
        m_totalEnemies = 3;
        m_spawnInterval = 2.0f;
        m_restDuration = 15.0f;
        m_hpBonus = 0;
        m_attackBonus = 0;
    } else if (wave == 2) {
        m_totalEnemies = 5;
        m_spawnInterval = 1.8f;
        m_restDuration = 15.0f;
        m_hpBonus = 10;
        m_attackBonus = 2;
    } else if (wave == 3) {
        m_totalEnemies = 7;
        m_spawnInterval = 1.5f;
        m_restDuration = 12.0f;
        m_hpBonus = 20;
        m_attackBonus = 4;
    } else if (wave == 4) {
        m_totalEnemies = 9;
        m_spawnInterval = 1.3f;
        m_restDuration = 12.0f;
        m_hpBonus = 35;
        m_attackBonus = 6;
    } else {
        m_totalEnemies = 10 + (wave - 4) * 2;
        m_spawnInterval = std::max(0.8f, 1.5f - (wave - 4) * 0.2f);
        m_restDuration = 8.0f;
        m_hpBonus = 35 + (wave - 4) * 15;
        m_attackBonus = 6 + (wave - 4) * 3;
    }
}

void SpawnManager::startRest() {
    m_state = WaveState::Resting;
    m_timer = m_restDuration;
}
