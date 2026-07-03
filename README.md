# Border Commander

2D indirect-control strategy game. You command a border village — build structures, recruit soldiers, and survive waves of enemies. Units fight autonomously; you make the macro decisions.

**Tech Stack:** C++20 / SFML 3.1.0 / Visual Studio 2022 / MSBuild

## Build

```
# Via Visual Studio
Open Border Commander.vcxproj → Build (Debug x64)

# Via command line
MSBuild.exe Border Commander.vcxproj -p:Configuration=Debug -p:Platform=x64 -t:Build -noautoresponse
```

Requires SFML 3.1.0 at `C:\SFML-3\SFML-3.1.0\`.

## How to Play

| Input | Action |
|-------|--------|
| **Left-click** | Select soldier / Click Barracks to recruit / Click tile to deselect |
| **Right-click** | Command selected soldier to move / Cancel placement mode |
| **1-4 keys** | Select building type from Build Menu |
| **Escape** | Cancel placement / Deselect |
| **R** | Restart after game over |
| **Drag (left mouse)** | Pan the map |

### Buildings

| Building | Cost | Size | Effect |
|----------|------|------|--------|
| HQ | Free | 2×2 | Starting building. If destroyed → game over. |
| Barracks | 80 Gold + 15 Food | 1×1 | Click to recruit soldiers. |
| Farm | 50 Gold | 1×1 | Produces 1.5 Food/sec. |
| Gold Mine | 50 Gold | 1×1 | Produces 0.8 Gold/sec. |

### Soldiers

- Cost: 40 Gold + 5 Food + 1 Population
- HP: 60, Attack: 10, Defense: 5
- Auto-engage enemies within 5 tiles
- Right-click to manually move

### Enemies

- Spawn in waves from map edges
- Auto-advance toward HQ
- Difficulty scales with each wave (more enemies, higher HP/ATK)
- Killing enemies grants bounty (25 + wave × 5 Gold)

### Victory / Defeat

- **Victory:** Survive 10 waves
- **Defeat:** HQ destroyed
- Press **R** to restart after either outcome

## Architecture

```
Layer 3:  Core        (Application, Game, Window, GameClock)
Layer 2:  Scene, UI   (SceneManager, GameScene, BuildMenu)
Layer 1:  Entity, AI, World, Manager
          (Entity/Building/Unit, StateMachine/States, TileMap/Camera/Pathfinder,
           SpawnManager, AudioManager)
Layer 0:  Event, Resource
          (EventBus, ResourceManager)
```

### Modules (14 phases)

| Phase | Module | Key Deliverables |
|-------|--------|-----------------|
| 0-3 | Core, Event, Resource | Game loop, scene system, event bus, resource manager |
| 4 | World | 30×20 tile map with terrain, camera panning, A* pathfinding |
| 5 | Entity | Entity base, Building system with production |
| 6 | Unit | Soldier recruitment, enemy spawning, movement |
| 7 | AI | FSM (Idle/MoveTo/Combat), autonomous behavior |
| 8 | Combat | Kill bounty, game over, damage flash, events |
| 9 | Wave System | SpawnManager with difficulty scaling |
| 10 | UI | BuildMenu panel with visual buttons |
| 11 | Victory | Win condition, stats screen, restart |
| 12 | Polish | Movement bounce, spawn animation, HP bars, AI labels |
| 13 | Audio | Event-driven sound effects, programmatic beeps |
| 14 | Delivery | README, balance tuning, cleanup |
