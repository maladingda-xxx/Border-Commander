#pragma once

enum class WaveState {
    Resting,
    Spawning,
    AwaitingClear
};

class SpawnManager {
public:
    SpawnManager();

    void update(float dt);

    bool shouldSpawn() const;
    void confirmSpawn();
    void notifyWaveCleared();

    WaveState getState() const { return m_state; }
    int getCurrentWave() const { return m_wave; }
    int getTotalEnemiesThisWave() const { return m_totalEnemies; }
    int getSpawnedThisWave() const { return m_spawnedCount; }
    float getRestTimeRemaining() const { return m_timer; }
    int getHPBonus() const;
    int getAttackBonus() const;

private:
    WaveState m_state = WaveState::Resting;
    int m_wave = 0;
    int m_totalEnemies = 0;
    int m_spawnedCount = 0;
    float m_timer = 0.0f;
    float m_spawnInterval = 2.0f;
    float m_restDuration = 0.0f;
    int m_hpBonus = 0;
    int m_attackBonus = 0;

    void startWave(int wave);
    void startRest();
};