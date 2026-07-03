# Border Commander 软件架构设计文档

> 文档版本：v1.0
> 创建日期：2026-07-03
> 项目类型：C++ 课程设计 / 2D 像素风策略模拟游戏
> 技术栈：C++20 / SFML 3.1.0 / Visual Studio 2022

---

## 目录

1. [项目需求分析](#1-项目需求分析)
2. [软件架构设计](#2-软件架构设计)
3. [系统目录结构设计](#3-系统目录结构设计)
4. [类职责分析](#4-类职责分析)
5. [面向对象设计](#5-面向对象设计)
6. [设计模式](#6-设计模式)
7. [开发路线](#7-开发路线)
8. [附录：现有项目问题诊断与修正建议](#8-附录)

---

## 1. 项目需求分析

### 1.1 游戏核心玩法

《Border Commander》是一款**间接控制型** 2D 策略模拟游戏。玩家扮演边境村庄指挥官，不直接操控任何单位，而是通过**建造、招募、调度**来影响战局。

**核心玩法循环：**

```
采集资源 → 建造/升级建筑 → 招募士兵 → 设置巡逻/防御路线 → 自动战斗 → 抵御敌人波次 → 获得奖励/解锁科技
```

**关键设计原则：**
- **玩家间接控制**：玩家下达宏观指令（建造、招募、派遣巡逻），士兵按照 AI 自主行动
- **自动化战斗**：单位靠近敌人后自动战斗，玩家不控制单个士兵
- **资源约束**：所有行动受资源限制，玩家需要平衡经济与军事
- **波次递增**：敌人波次逐渐变强，形成越来越大的防守压力

### 1.2 MVP（最小可运行版本）

MVP 阶段应展示核心玩法循环的完整闭环：

| 功能模块 | MVP 必须包含 | 说明 |
|---------|------------|------|
| **地图** | 固定大小的 2D 网格地图 | 约 30×20 格，可用数组/向量存储 |
| **地形** | 草地（可行走）、障碍物（不可行走） | 两种地形即可 |
| **建筑** | 指挥部（初始）、兵营（招募）、农场（食物）、金矿（金币） | 4 种建筑覆盖核心经济-军事链条 |
| **资源** | 金币、食物、人口 | 金币用于建造/招募，食物支撑人口，人口限制兵力 |
| **士兵** | 一种近战士兵 | MVP 只需要一种兵种 |
| **敌人** | 一种近战敌人 | 从地图边缘生成，向村庄进攻 |
| **AI** | 基础状态机（空闲→巡逻→战斗→返回） | 4 个状态足够展示 AI 设计 |
| **战斗** | 简单的数值结算（攻击力-防御力，击杀/死亡） | 不需要复杂公式 |
| **胜利条件** | 存活 N 波敌人 | 测试玩法可行性 |
| **UI** | HUD 显示资源、建筑菜单（按钮） | 基础交互 |

**MVP 不包含（后续扩展）：**
- 多兵种、科技树、升级系统
- 复杂动画（使用色块代替精灵图也行）
- 存档/读档
- 音效/音乐
- 菜单/设置界面
- 粒子特效

### 1.3 后续可扩展玩法

以下功能在设计阶段预留接口，MVP 后按优先级迭代：

| 优先级 | 功能 | 依赖 |
|-------|------|------|
| 高 | 多兵种（弓箭手、骑兵、法师） | 兵营 + 科技建筑 |
| 高 | 多种敌人类型（快速型、坦克型、飞行型） | 敌人波次系统 |
| 高 | 建筑升级 | 资源系统 |
| 中 | 科技树 | 科技建筑 |
| 中 | 英雄单位（可升级，不可直接控制但拥有技能） | 兵营系统 |
| 中 | 存档/读档 | 序列化系统 |
| 中 | 动画系统（精灵图 + 帧动画） | 资源加载 |
| 中 | 音效/音乐 | 音频库 |
| 低 | 多关卡/地图 | 地图加载 |
| 低 | 天气/昼夜系统 | 视觉效果层 |
| 低 | 成就系统 | 事件系统 |
| 低 | 本地化 | 字符串表 |

### 1.4 游戏循环（Game Loop）

采用**固定时间步长 + 可变渲染**的经典游戏循环：

```
+--------------------------------------------------------+
|  Game Loop                                             |
|                                                        |
|  1. processInput()     // 处理 SFML 事件、输入映射      |
|  2. update(deltaTime)  // 固定逻辑步长 (如 60Hz)        |
|     ├─ updateAI()       // 所有 AI 单位决策             |
|     ├─ updateEntities() // 实体状态更新                 |
|     ├─ updateWorld()    // 世界状态更新（资源生产等）     |
|     ├─ updateCombat()   // 战斗结算                     |
|     └─ updateUI()       // UI 状态更新                  |
|  3. render()            // 可变帧率渲染                  |
|     ├─ renderWorld()    // 地图 & 地形                  |
|     ├─ renderEntities() // 建筑 & 单位                  |
|     ├─ renderEffects()  // 特效层                       |
|     └─ renderUI()       // HUD & 菜单                   |
+--------------------------------------------------------+
```

**设计决策：**
- **固定逻辑步长**确保游戏在不同设备上表现一致（避免物理/逻辑受帧率影响）
- **可变渲染**充分利用显示器刷新率
- 使用 `std::chrono` 或 SFML 的 `sf::Clock` 进行时间测量
- 滞后累积（lag accumulation）处理帧率波动，保证逻辑不会因渲染慢而跳跃

**现有项目问题诊断：** 当前 `main.cpp` 使用了最简单的帧率依赖循环（每帧固定步进），且 `main.cpp` 错误地放在了 `assets/` 目录下。源码应放在 `src/` 目录。

---

## 2. 软件架构设计

### 2.1 模块划分

整个项目划分为 **10 个核心模块**，按职责和依赖关系分层组织：

```
┌──────────────────────────────────────────────────────────────┐
│                        Layer 3: Application                  │
│  ┌──────────┐                                                │
│  │   Core   │  Application, Game, Engine                     │
│  └────┬─────┘                                                │
│       │  owns                                                │
├───────┼──────────────────────────────────────────────────────┤
│       │              Layer 2: Systems & Logic                │
│  ┌────┴─────┐  ┌──────────┐  ┌──────────┐  ┌──────────────┐ │
│  │  Scene   │  │  World   │  │  Entity  │  │     AI       │ │
│  └────┬─────┘  └────┬─────┘  └────┬─────┘  └──────┬───────┘ │
│       │              │             │               │         │
├───────┼──────────────┼─────────────┼───────────────┼─────────┤
│       │              │    Layer 1: Foundation                 │
│  ┌────┴─────┐  ┌────┴─────┐  ┌────┴─────┐  ┌──────┴───────┐ │
│  │  Event   │  │ Manager  │  │ Graphics │  │     Math     │ │
│  └──────────┘  └──────────┘  └──────────┘  └──────────────┘ │
└──────────────────────────────────────────────────────────────┘
```

#### 模块详细说明

##### 2.1.1 Core 模块

- **职责**：应用程序入口、游戏循环、引擎生命周期管理
- **包含类**：`Application`、`Game`、`Engine`
- **依赖**：所有其他模块（作为组装层）
- **为什么这样划分**：Core 是整个程序的"胶水层"，负责创建和装配所有子系统，但自身不包含游戏逻辑。这是**依赖注入**思想的体现——Core 知道所有模块，但各模块不知道 Core 的存在。

##### 2.1.2 Scene 模块

- **职责**：场景抽象、场景管理、场景切换
- **包含类**：`Scene`（抽象基类）、`SceneManager`
- **依赖**：Event 模块（接收场景切换事件）
- **为什么这样划分**：游戏通常有多个"画面"（主菜单、游戏内、暂停、结算），每个画面都是独立的场景。场景模式允许各画面独立开发、测试，互不干扰。Scene 作为基类定义了统一的 `enter()`、`exit()`、`update()`、`render()` 接口。

##### 2.1.3 World 模块

- **职责**：网格地图、地形系统、路径寻路
- **包含类**：`Tile`、`TileMap`、`Pathfinder`
- **依赖**：Math 模块
- **为什么这样划分**：地图是策略游戏的核心空间结构，独立为一个模块便于替换地图生成算法、添加地形类型、扩展寻路策略。

##### 2.1.4 Entity 模块

- **职责**：所有游戏对象（建筑、单位、资源点）
- **包含类**：`Entity`（抽象基类）、`Building`、`Unit`、`Soldier`、`Enemy`、`Resource`
- **依赖**：World 模块（获取位置/格信息）、Graphics 模块（精灵组件）
- **为什么这样划分**：实体是游戏中最复杂、类型最多的系统。采用**组合优于继承**的设计——Entity 持有组件（位置组件、渲染组件、战斗组件），具体行为由组件和 AI 驱动。这样避免了深层继承带来的"钻石问题"和"上帝基类"。

##### 2.1.5 AI 模块

- **职责**：单位行为决策、有限状态机、寻路请求
- **包含类**：`State`（接口）、`StateMachine`、具体状态类（`IdleState`、`PatrolState`、`CombatState`、`FleeState`）
- **依赖**：Entity 模块（读取/修改单位状态）、World 模块（寻路/空间查询）
- **为什么这样划分**：AI 是游戏的核心亮点之一。独立模块化意味着可以独立测试 AI 逻辑、替换决策模型（从 FSM 升级到行为树），而不影响其他系统。AI 模块通过接口与 Entity 交互，不直接依赖具体实体类型。

##### 2.1.6 Manager 模块

- **职责**：横切关注点的集中管理（资源、输入、音频、事件）
- **包含类**：`ResourceManager`、`InputManager`、`AudioManager`、`ConfigManager`、`SpawnManager`
- **依赖**：通常只依赖底层模块或第三方库
- **为什么这样划分**：Manager 类处理的是贯穿多个模块的横切关注点。将它们集中管理而非散布在各处，便于维护和替换实现。**但要注意**：Manager 不应成为"万能类"，每个 Manager 只负责一个明确的职责（单一职责原则）。

##### 2.1.7 UI 模块

- **职责**：用户界面元素（按钮、面板、HUD、菜单）
- **包含类**：`UIElement`（抽象基类）、`Button`、`Label`、`Panel`、`HUD`、`BuildMenu`
- **依赖**：Graphics 模块（渲染）、Event 模块（发送UI事件）
- **为什么这样划分**：UI 在游戏中自成体系，需要处理鼠标/键盘事件、布局、层级关系。独立模块便于替换 UI 框架（从自绘到 ImGui 等）、添加 UI 动画。

##### 2.1.8 Graphics 模块

- **职责**：渲染抽象、精灵管理、相机、动画播放
- **包含类**：`Renderer`、`Sprite`、`Animation`、`Camera`
- **依赖**：SFML（第三方库）、Math 模块
- **为什么这样划分**：将 SFML 的渲染细节封装在此模块内，其他模块不直接接触 SFML API。这样做的好处是：如果将来换渲染库，只需修改此模块。

##### 2.1.9 Event 模块

- **职责**：模块间松耦合通信
- **包含类**：`EventBus`、`Event`（基类）、各种具体事件类
- **依赖**：无（纯逻辑模块）
- **为什么这样划分**：游戏系统中，许多事件需要跨模块通知（如"建筑建造完成"需要通知 UI 刷新、资源系统调整、音效播放）。EventBus 解耦了事件的发送者和接收者，符合**开闭原则**——添加新的事件监听者不需要修改事件发送者。

##### 2.1.10 Math 模块

- **职责**：数学工具（向量运算、矩形、随机数）
- **包含类**：`Vector2`、`Rect`、`Random`
- **依赖**：无（纯工具模块）
- **为什么这样划分**：虽然 SFML 自带数学类型，但封装一层可以隔离第三方依赖、添加项目特有的数学工具函数。也可以直接使用 SFML 的 `sf::Vector2f`/`sf::Vector2i`，视团队偏好而定。**架构建议**：MVP 阶段直接使用 SFML 的数学类型以减少代码量，后期如果需要自定义行为再封装。

### 2.2 模块通信机制

模块之间通过以下三种机制通信：

| 通信方式 | 使用场景 | 示例 |
|---------|---------|------|
| **直接方法调用** | 明确的依赖关系（Module A 持有 Module B 的引用） | Scene 调用 Entity 的 update() |
| **EventBus** | 松耦合的跨模块通知 | "敌人被杀"事件通知 UI 更新击杀计数、音效播放 |
| **回调/观察者** | 一对一或一对多的状态监听 | UI 监听资源变化以更新 HUD 显示 |

**通信原则：**
- **高层模块可以依赖低层模块**（Scene 可以调用 World 和 Entity）
- **低层模块不能依赖高层模块**（World 不知道 Scene 的存在）
- **同层模块间通过 EventBus 通信**（AI 不直接调用 UI，而是发事件）
- **Core 模块是唯一的组装层**，知道所有模块并建立它们之间的连接

### 2.3 架构设计决策与讨论

#### 决策 1：不使用完整 ECS，采用"组件化实体"折中方案

**纯 ECS（Entity-Component-System）** 将所有数据放在 Component 中，System 遍历 Component 处理逻辑。虽然现代、高效，但对于课程设计来说：
- ECS 弱化了 OOP 概念（继承、多态），与课程目标"展示 OOP"冲突
- 完整 ECS 需要大量样板代码，增加复杂度

**采用方案**：`Entity` 持有若干组件（通过组合），同时允许继承层次（`Entity → Unit → Soldier`）。这样既展示了继承和多态，又避免了深层继承的弊端。

```text
Entity（抽象基类）
├─ 拥有 TransformComponent（位置、旋转）
├─ 拥有 RenderComponent（精灵、动画）
├─ 拥有 CollisionComponent（碰撞体积）
│
├─ Building（继承 Entity）
│   └─ 拥有 ProductionComponent（资源产出）
│
└─ Unit（继承 Entity）
    ├─ 拥有 CombatComponent（攻击力、防御力、生命值）
    ├─ 拥有 MovementComponent（速度、目标位置）
    ├─ 拥有 AIComponent（状态机引用）
    │
    ├─ Soldier（继承 Unit）
    └─ Enemy（继承 Unit）
```

#### 决策 2：Manager 使用服务定位器模式而非纯 Singleton

纯 Singleton 的测试困难（全局状态）和隐式依赖问题广为人知。**采用方案**：创建一个 `ServiceLocator` 类，在 Core 初始化时注册所有 Manager，各模块通过 `ServiceLocator::get<ResourceManager>()` 获取服务。这比 Singleton 更容易在测试中替换实现。

MVP 阶段可以简化为 Singleton（快速开发），但架构设计层面预留 ServiceLocator 接口。

#### 决策 3：配置驱动而非硬编码

建筑属性、单位属性、敌人波次信息不硬编码在 C++ 中，而是存储在 JSON 配置文件中（`assets/config/`）。`ConfigManager` 负责加载和解析。这样：
- 数值调整不需要重新编译
- 策划人员（或开发者自己调试）可以直接修改 JSON
- 符合数据驱动设计理念

---

## 3. 系统目录结构设计

### 3.1 完整目录树

```text
Border Commander/
├── assets/                          # 游戏资源（运行时加载）
│   ├── textures/                    # 纹理/精灵图
│   │   ├── buildings/               # 建筑精灵
│   │   ├── units/                   # 单位精灵
│   │   ├── terrain/                 # 地形精灵
│   │   └── ui/                      # UI精灵
│   ├── audio/                       # 音频文件
│   │   ├── sfx/                     # 音效
│   │   └── music/                   # 背景音乐
│   ├── fonts/                       # 字体文件
│   ├── maps/                        # 地图数据文件 (JSON/Tiled)
│   └── config/                      # 游戏配置文件 (JSON)
│       ├── buildings.json           # 建筑属性表
│       ├── units.json               # 单位属性表
│       ├── waves.json               # 敌人波次配置
│       └── settings.json            # 游戏设置默认值
│
├── include/                         # 公共头文件 (.h/.hpp)
│   ├── Core/                        # 核心模块
│   ├── World/                       # 世界/地图模块
│   ├── Entity/                      # 实体模块
│   ├── AI/                          # AI模块
│   ├── Manager/                     # 管理器模块
│   ├── UI/                          # UI模块
│   ├── Scene/                       # 场景模块
│   ├── Graphics/                    # 图形模块
│   ├── Event/                       # 事件模块
│   └── Math/                        # 数学模块
│
├── src/                             # 实现文件 (.cpp)
│   ├── Core/
│   ├── World/
│   ├── Entity/
│   ├── AI/
│   ├── Manager/
│   ├── UI/
│   ├── Scene/
│   ├── Graphics/
│   ├── Event/
│   └── Math/
│
├── docx/                            # 设计文档
│   └── BorderCommander-Software-Architecture-Design.md
│
├── lib/                             # 第三方库（手动管理或 vcpkg）
│   └── SFML-3.1.0/                  # SFML 库文件
│
├── test/                            # 单元测试（可选阶段添加）
│   └── ...
│
├── .gitignore                       # Git 忽略规则
├── README.md                        # 项目说明
└── Border Commander.vcxproj         # VS 项目文件
```

### 3.2 目录职责说明

| 目录 | 职责 | 说明 |
|------|------|------|
| `assets/` | 运行时加载的资源文件 | 与源码分离，便于打包和替换。**不应包含任何 .cpp/.h 文件** |
| `include/` | 公共头文件 | 每个子目录对应一个模块，包含该模块的公开接口 |
| `src/` | 实现文件 | 与 `include/` 子目录一一对应 |
| `docx/` | 设计文档 | 需求文档、架构文档、会议记录 |
| `lib/` | 第三方依赖 | 项目依赖的外部库。SFML 已安装在系统路径 (`C:\SFML-3\`)，因此此处可为空或使用 vcpkg |
| `test/` | 测试代码 | 单元测试、集成测试（课程设计可选，但体现工程意识） |
| `.gitignore` | 版本控制忽略规则 | 忽略 `x64/`、`.vs/`、`*.user` 等生成文件 |

### 3.3 .gitignore 建议内容

```gitignore
# Visual Studio
.vs/
x64/
*.user
*.suo
*.sdf
*.opendb
*.db

# Build outputs
Debug/
Release/
*.exe
*.dll
*.lib
*.pdb
*.ilk
*.obj
*.tlog
*.recipe
*.log

# OS
.DS_Store
Thumbs.db
```

---

## 4. 类职责分析

### 4.1 Core 模块

| 类名 | 职责 | 为什么存在 |
|------|------|-----------|
| `Application` | 程序入口对象的封装。创建窗口、初始化引擎、启动游戏循环 | 将 `main()` 的职责最小化——`main()` 只创建 `Application` 并调用 `run()` |
| `Engine` | 子系统初始化和生命周期管理。持有 ServiceLocator | 集中管理所有 Manager 的创建、初始化、销毁顺序。避免在 Game 中堆砌初始化代码 |
| `Game` | 游戏主循环。协调 Scene、更新逻辑、渲染 | 核心调度类。拥有 SceneManager，驱动 update/render 循环 |

### 4.2 Scene 模块

| 类名 | 职责 | 为什么存在 |
|------|------|-----------|
| `Scene`（抽象基类） | 定义场景生命周期接口：`onEnter()`、`onExit()`、`update(dt)`、`render(window)` | 多态的场景切换。所有具体场景（主菜单、游戏内、暂停）实现此接口 |
| `SceneManager` | 管理场景栈（scene stack）、处理场景切换请求 | 支持 push/pop 场景（如暂停菜单覆盖在游戏场景之上），而非简单替换 |

### 4.3 World 模块

| 类名 | 职责 | 为什么存在 |
|------|------|-----------|
| `Tile` | 单个地图格子的数据：地形类型、是否可行走、占用状态 | 地图的最小数据单元 |
| `TileMap` | 二维网格容器。提供格子查询、邻居查询、AABB范围内的格子遍历 | 封装二维数组，提供便捷的空间查询接口 |
| `Pathfinder` | A* 寻路算法实现 | 士兵和敌人需要寻路到目标位置。独立为类便于替换算法 |

### 4.4 Entity 模块

| 类名 | 职责 | 为什么存在 |
|------|------|-----------|
| `Entity`（抽象基类） | 所有游戏对象的基类。提供唯一ID、激活状态、位置、渲染接口 | 统一管理所有"存在于游戏世界中的东西"，便于遍历和查询 |
| `TransformComponent` | 位置、朝向、缩放 | 组合组件，可被 Entity 持有 |
| `RenderComponent` | 精灵引用、渲染层级、颜色调制 | 组合组件，将"外观"与"逻辑"分离 |
| `CombatComponent` | 攻击力、防御力、生命值、攻击间隔 | 组合组件，只被 Unit 持有 |
| `MovementComponent` | 移动速度、目标位置、路径 | 组合组件，只被 Unit 持有 |
| `ProductionComponent` | 资源产出类型、产出速率 | 组合组件，只被生产类 Building 持有 |
| `Unit`（继承 Entity） | 可移动、可战斗的游戏对象基类 | 士兵和敌人的共同父类，封装 Move/Attack 接口 |
| `Soldier`（继承 Unit） | 玩家士兵。附加巡逻行为、玩家标识 | 与 Enemy 区分阵营，可被招募和指派 |
| `Enemy`（继承 Unit） | 敌人单位。附加波次信息、奖励金币 | 与 Soldier 区分阵营，AI 行为不同（主动进攻） |
| `Building`（继承 Entity） | 建筑的共同基类。建造进度、占地大小 | 区别于 Unit——不可移动，可被建造/拆除 |
| `Headquarters`（继承 Building） | 指挥部。初始建筑，被摧毁则游戏失败 | 胜利条件的关键建筑 |
| `Barracks`（继承 Building） | 兵营。可招募士兵 | 兵力来源 |
| `Farm`（继承 Building） | 农场。产出食物 | 食物来源，支撑人口 |
| `GoldMine`（继承 Building） | 金矿。产出金币 | 金币来源 |
| `Wall`（继承 Building） | 城墙。阻挡敌人移动 | 防御性建筑 |

### 4.5 AI 模块

| 类名 | 职责 | 为什么存在 |
|------|------|-----------|
| `IState`（接口） | 定义状态接口：`enter()`、`execute(dt)`、`exit()` | 状态模式的核心抽象。每个具体状态实现此接口 |
| `StateMachine` | 管理当前状态、执行状态转换 | 封装 FSM 逻辑：当前状态执行、条件判断、状态切换。被 Unit 持有 |
| `IdleState` | 空闲状态：原地等待，周期性扫描范围内敌人 | 默认状态 |
| `PatrolState` | 巡逻状态：沿设定路径移动 | 玩家给士兵指派的巡逻任务 |
| `CombatState` | 战斗状态：移动到敌人附近并攻击 | 检测到敌人后自动进入 |
| `FleeState` | 逃跑状态：生命值过低时撤退 | 提高单位生存率 |
| `MoveToState` | 移动到目标位置 | 通用移动状态 |

### 4.6 Manager 模块

| 类名 | 职责 | 为什么存在 |
|------|------|-----------|
| `ResourceManager` | 管理玩家资源（金币、食物、人口）。提供增减/查询接口 | 集中管理经济系统，发出资源变化事件通知 UI |
| `InputManager` | 封装 SFML 输入。将按键/鼠标映射为游戏命令 | 解耦物理输入与游戏逻辑。NPC测试时可直接注入命令 |
| `AudioManager` | 音效和背景音乐播放 | 封装 SFML 音频 API |
| `ConfigManager` | 加载和缓存 JSON 配置文件。提供类型安全的查询接口 | 数据驱动的核心。避免配置硬编码 |
| `SpawnManager` | 管理敌人波次生成逻辑 | 波次系统的核心。控制生成时机、数量和类型 |
| `ServiceLocator` | 提供全局服务注册和查找 | 替代 Singleton 的服务定位器。便于测试时替换服务 |

### 4.7 UI 模块

| 类名 | 职责 | 为什么存在 |
|------|------|-----------|
| `UIElement`（抽象基类） | UI 组件基类：位置、大小、可见性、事件处理 | 统一 UI 接口，支持构建层级 UI |
| `Button` | 可点击按钮。响应点击事件 | 建造菜单、招募按钮 |
| `Label` | 文本显示 | 资源数值显示、单位名称 |
| `Panel` | 矩形面板，可作为其他元素的容器 | 信息面板、菜单背景 |
| `HUD` | 游戏内抬头显示。管理资源条、小地图、选中信息 | 游戏界面的主 UI 层 |
| `BuildMenu` | 建造菜单。显示可建造建筑列表 | 玩家主要的交互界面之一 |
| `UIManager` | UI 元素管理和渲染调度 | 管理 UI 元素的添加、移除、层级排序、事件分发 |

### 4.8 Graphics 模块

| 类名 | 职责 | 为什么存在 |
|------|------|-----------|
| `Renderer` | 渲染调度器。持有 SFML RenderWindow 引用 | 封装渲染流程，管理渲染状态 |
| `Camera` | 视口控制。平移、缩放 | 支持地图滚动和缩放（滚轮） |
| `Animation` | 帧动画数据。帧序列、时间间隔、循环模式 | 精灵动画的基础 |
| `Sprite` | 精灵封装。关联纹理、设置纹理区域、颜色调制 | 封装 SFML Sprite 操作 |

### 4.9 Event 模块

| 类名 | 职责 | 为什么存在 |
|------|------|-----------|
| `Event`（基类） | 事件基类。提供类型标识 | 所有事件的共同父类 |
| `EventBus` | 事件总线。订阅、取消订阅、发布事件 | 核心解耦机制。模块间异步通信的唯一渠道 |
| 具体事件类 | 如 `ResourceChangedEvent`、`EntityDestroyedEvent`、`WaveStartEvent`、`BuildingPlacedEvent` | 每种事件携带其特有的数据 |

### 4.10 Math 模块

| 类名 | 职责 | 为什么存在 |
|------|------|-----------|
| `Random` | 随机数工具。提供范围内的随机整数/浮点数 | 集中管理随机数生成器（Mersenne Twister），便于复现和测试 |

> **Math 模块注意**：MVP 阶段可直接使用 SFML 的 `sf::Vector2f`、`sf::Vector2i`、`sf::FloatRect` 等类型，减少不必要的封装。`Random` 工具类值得保留。

---

## 5. 面向对象设计

### 5.1 封装（Encapsulation）

**使用场景：** 所有类的数据成员。

**原则：** 所有数据成员为 `private`，通过 `public`/`protected` 方法访问。不提供"裸"的 getter/setter 对所有成员——只为真正需要外部访问的成员提供访问方法。

```text
好的封装示例：
  Unit::takeDamage(int amount)    // 语义化方法，而非 setHealth(getHealth() - amount)
  ResourceManager::canAfford()    // 查询方法，而非直接暴露 gold 成员

存在原因：
  数据成员变 private 使得修改内部实现不影响外部代码。
  例如：生命值从 int 改为 float 时，只需修改 Unit 内部实现。
```

### 5.2 继承（Inheritance）

**使用场景：**

| 继承关系 | 基类 | 派生类 | 说明 |
|---------|------|--------|------|
| 实体继承 | `Entity` | `Unit`, `Building` | `Unit` 和 `Building` 是游戏中两种根本不同的对象 |
| 单位继承 | `Unit` | `Soldier`, `Enemy` | 共享移动、战斗逻辑，但 AI 和阵营不同 |
| 建筑继承 | `Building` | `Headquarters`, `Barracks`, `Farm`, `GoldMine`, `Wall` | 共享建筑属性，各自实现特有逻辑 |
| 状态继承 | `IState` | `IdleState`, `PatrolState`, `CombatState`, `FleeState` | 状态模式的核心 |
| UI 继承 | `UIElement` | `Button`, `Label`, `Panel` | UI 组件层级 |
| 组件继承 | `Component` | `TransformComponent` 等 | 组合组件的统一接口 |

**设计原则：**
- 继承层次不超过 3 层（当前最深：`Entity → Unit → Soldier`，刚好 3 层）
- 基类析构函数为 `virtual`（确保多态删除安全）
- 使用 `override` 关键字明确标记重写
- 只在"是一个（is-a）"关系中使用继承，绝不为代码复用而继承

### 5.3 多态（Polymorphism）

**使用场景：**

| 场景 | 多态形式 | 说明 |
|------|---------|------|
| Entity 更新 | `Entity::update(dt)` 虚函数 | Scene 遍历 `vector<unique_ptr<Entity>>`，调用 `update()`，每个子类有不同行为 |
| Entity 渲染 | `Entity::render(window)` 虚函数 | 多态的渲染调用 |
| AI 状态 | `IState::execute(dt)` 虚函数 | StateMachine 持有 `unique_ptr<IState>`，运行时切换状态，多态调用 |
| UI 事件 | `UIElement::onClick()` 虚函数 | UIManager 遍历 UI 元素，多态分发点击事件 |

**为什么使用多态：**
- Entity 列表是多态的：`vector<unique_ptr<Entity>>` 可以同时存储 Soldier、Enemy、Building
- 状态机是多态的：`unique_ptr<IState>` 在运行时指向不同具体状态
- 这使得添加新实体类型、新状态、新 UI 组件不需要修改使用方代码（开闭原则）

### 5.4 组合（Composition）

**使用场景：**

| 组合关系 | 所有者 → 组成部分 | 说明 |
|---------|------------------|------|
| Game → SceneManager | `Game` 拥有 `SceneManager` | Game 生命周期内独占 SceneManager |
| Entity → Component | `Entity` 拥有 `TransformComponent`, `RenderComponent` 等 | Entity 销毁时组件一同销毁（强的拥有关系） |
| Unit → StateMachine | `Unit` 拥有 `StateMachine` | 每个单位各有自己的 AI 状态机 |
| Renderer → Camera | `Renderer` 拥有 `Camera` | 每个渲染器关联一个相机 |

**为什么使用组合：**
- 组合表达了"有一个（has-a）"或"部分-整体（part-of）"关系
- 比继承更灵活——组件的组合方式可以动态改变
- 避免继承带来的紧耦合

**组合 VS 继承的权衡：**

项目中同时使用组合和继承：
- **继承用在**："是一个"关系（Soldier 是一个 Unit，Barracks 是一个 Building）
- **组合用在**："有"关系（Entity 有 TransformComponent，Unit 有 CombatComponent）
- **避免的陷阱**：不使用巨大的"God Entity"基类包含所有可能的属性，而是通过组件按需组合

### 5.5 聚合（Aggregation）

**使用场景（较弱的拥有关系）：**

| 聚合关系 | 容器 → 被包含对象 | 说明 |
|---------|------------------|------|
| TileMap → Tile | `TileMap` 包含 `Tile` 对象的网格 | Tile 可以独立于 TileMap 存在（复制、序列化） |
| Animation → Frame | `Animation` 包含帧序列 | Frame 是独立的数据结构 |
| HUD → UIElement | `HUD` 持有 UI 元素的指针/引用 | UI 元素在 HUD 之外创建，可复用 |

**为什么使用聚合而非组合：**
- 被包含的对象生命周期不依赖容器（Tile 可以在没有 TileMap 的情况下存在）
- 被包含的对象可以被多个地方引用

### 5.6 依赖（Dependency）

**使用场景（临时的使用关系）：**

| 依赖关系 | 说明 |
|---------|------|
| `Pathfinder` 依赖 `TileMap` 只读引用 | Pathfinder 使用 TileMap 查询格子信息，但不拥有它 |
| `StateMachine` 依赖 `Unit` 引用 | 状态机需要读取/修改所属单位的状态 |
| `SpawnManager` 依赖 `TileMap` 和 `Entity` 列表 | 生成逻辑需要地图信息和实体容器的引用 |

**为什么是依赖而非拥有：**
- 这些对象由其他地方创建和管理
- 依赖方只需要"使用"而非"拥有"被依赖方
- 通过接口或 const 引用传递，降低耦合

**依赖关系汇总图：**

```text
Game ───owns──→ SceneManager ───owns──→ Scene (polymorphic)
                  │
Scene (GameScene) ───depends──→ TileMap
                  │──depends──→ Entity list
                  │──depends──→ UI elements
                  │──depends──→ SpawnManager
                  │──depends──→ Pathfinder
                  │
Entity ───composes──→ TransformComponent, RenderComponent
Unit ───composes───→ CombatComponent, MovementComponent
Unit ───owns───────→ StateMachine
StateMachine ───depends──→ IState (polymorphic)
StateMachine ───depends──→ Unit (reference)
```

---

## 6. 设计模式

### 6.1 状态模式（State Pattern）

**适用位置：** AI 模块 - 单位行为

**为什么：**
- 单位的行为因环境而变化（空闲、巡逻、战斗、逃跑）
- 每种行为的进入条件、执行逻辑、退出动作各不相同
- 行为之间可以互相转换（战斗 → 逃跑 当生命值过低）
- 添加新行为不影响现有代码

**结构：**
- `IState` 接口定义 `enter()`、`execute(dt)`、`exit()`
- `StateMachine` 持有当前状态，执行转换逻辑
- 各具体状态实现自己的行为逻辑

**替代方案讨论：** 行为树（Behavior Tree）比状态机更灵活，但 MVP 阶段状态机足够。架构设计上，`IState` 接口可以平滑过渡到行为树节点接口。

### 6.2 观察者模式（Observer Pattern）/ 事件总线

**适用位置：** Event 模块 - EventBus

**为什么：**
- 一个事件需要通知多个系统（如"建筑建造完成"需要：更新资源、刷新 UI、播放音效）
- 事件发送者不需要知道谁在监听
- 新增监听者不需要修改事件发送者
- 支持一对多通信

**结构：**
- `EventBus` 维护 `map<EventType, vector<Callback>>`
- 各模块订阅感兴趣的事件类型
- 事件触发时，EventBus 遍历并调用所有订阅者的回调
- 使用 `std::function` 作为回调类型，支持 lambda、成员函数指针、自由函数

**替代方案讨论：** 简单的函数回调链（信号槽）也可以实现类似效果。EventBus 的优势在于类型安全和中心化管理。

### 6.3 单例模式（Singleton Pattern）/ 服务定位器（Service Locator）

**适用位置：** Manager 模块 - ServiceLocator

**为什么：**
- 某些服务需要在全局被访问（ResourceManager、InputManager、AudioManager）
- 通过参数传递方式对每个类注入依赖会导致构造函数参数过多
- 但纯 Singleton 引入全局可变状态，测试困难

**MVP 阶段折中方案：**
- 使用 ServiceLocator 而非直接 Singleton
- 在测试中可以为 ServiceLocator 注册 Null 实现或 Mock 实现
- 各模块通过 `ServiceLocator::get<T>()` 获取服务，而非直接调用 `T::getInstance()`

**设计建议：** `ServiceLocator` 的 `get<T>()` 在 Release 模式返回"真"服务，在 Debug 模式可配置为返回替代实现。这是"依赖注入的穷人版本"。

### 6.4 工厂模式（Factory Pattern）

**适用位置：** Entity 模块 - 实体创建

**为什么：**
- 实体创建过程可能涉及多个步骤（分配 ID、设置组件、注册回调）
- 集中管理创建逻辑，避免散落在各处
- 便于添加创建前/后处理（如日志记录、事件广播）

**结构：**
- `EntityFactory` 提供 `createSoldier(type, position)`、`createEnemy(type, position)` 等方法
- Building 创建类似：`EntityFactory::createBuilding(type, tilePosition)`
- Factory 读取 ConfigManager 获取默认属性，组装组件

### 6.5 策略模式（Strategy Pattern）

**适用位置：** 战斗计算、资源产出计算

**为什么：**
- 不同建筑有不同的资源产出公式
- 不同单位有不同的伤害计算方式
- 策略可以被替换而无需修改使用者代码

**结构（MVP 后可优化为此模式）：**
- `IProductionStrategy` 接口，不同建筑类型实现不同产出公式
- `ICombatStrategy` 接口，不同单位实现不同战斗逻辑

**MVP 简化：** MVP 阶段建筑类型少，可在 Entity 子类中直接实现产出逻辑。当建筑类型超过 5 种时，重构为策略模式。

### 6.6 命令模式（Command Pattern）

**适用位置：** 玩家操作

**为什么：**
- 玩家操作（建造、招募、设置巡逻）需要被统一管理
- 可以记录操作历史（用于将来的撤销功能）
- 可以将操作序列化（用于将来的回放功能）
- 输入映射：不同按键绑定到不同命令

**结构：**
- `ICommand` 接口定义 `execute()` 和（可选）`undo()`
- `BuildCommand`、`RecruitCommand`、`SetPatrolCommand` 等具体命令
- `CommandQueue` 管理命令队列

**MVP 简化：** MVP 阶段可不实现完整命令模式，而是用简单的回调处理 UI 按钮点击。在 UI 按钮响应函数中直接调用 `ResourceManager` 和实体创建逻辑即可。当需要撤销/回放功能时再重构。

### 6.7 设计模式汇总表

| 模式 | 模块 | 优先级 | MVP 是否实现 |
|------|------|--------|------------|
| State | AI | **必须** | 是 |
| Observer (EventBus) | Event | **必须** | 是 |
| Service Locator | Manager | **必须** | 是 |
| Factory | Entity | **推荐** | 简化版 |
| Strategy | Entity/Combat | 可选 | 否（直接实现） |
| Command | Input | 可选 | 否（简化回调） |
| Object Pool | Entity | 可选 | 否 |
| Mediator | UI | 可选 | 否 |

---

## 7. 开发路线

### 总体原则

- **每阶段独立可运行**：每个 Phase 结束时，项目可以编译、运行、展示阶段性成果
- **每阶段一个 Git Commit**：粒度合理，便于回退和评审
- **先骨架后血肉**：先搭模块框架和接口，再填充实现细节
- **纵向切片**：每个 Phase 完成一个完整的功能切片，而非横向做完所有"层"

### Phase 0：项目基础设施（预计：0.5天）

**目标：** 清理并规范化现有项目结构

**任务：**
1. 将 `assets/main.cpp` 移动到 `src/main.cpp`（修正文件位置）
2. 创建 `include/` 子目录结构（Core、World、Entity 等空目录）
3. 创建 `src/` 子目录结构（与 include 对应）
4. 更新 `.vcxproj` 的 Include 路径和源文件引用
5. 将 C++ 标准从 C++17 升级为 C++20（修改 `LanguageStandard` 为 `stdcpp20`）
6. 创建 `.gitignore` 文件
7. 初始化 Git 仓库，完成初始 Commit

**可运行验证：** 编译通过，窗口正常显示（保持原有 SFML 窗口 demo）

### Phase 1：核心框架与主循环（预计：1天）

**目标：** 建立游戏引擎骨架，实现标准游戏循环

**新增类：** `Application`、`Engine`、`Game`

**任务：**
1. 实现 `Application` 类（封装 `main` 函数逻辑）
2. 实现 `Game` 类（固定时间步长游戏循环，记录 FPS）
3. 实现 `Engine` 类（子系统初始化框架，当前为空）

**可运行验证：** 窗口显示 FPS 文本，游戏循环正常运行

### Phase 2：场景系统（预计：0.5天）

**目标：** 实现场景管理和切换

**新增类：** `Scene`（抽象基类）、`SceneManager`

**任务：**
1. 定义 `Scene` 接口
2. 实现 `SceneManager` 的场景栈
3. 创建一个简单的 `GameScene`（当前为空场景）

**可运行验证：** 场景系统工作正常，可创建空场景

### Phase 3：资源管理与事件系统（预计：1天）

**目标：** 建立模块通信基础设施

**新增类：** `EventBus`、基础事件类型、`ResourceManager`、`ConfigManager`、`ServiceLocator`

**任务：**
1. 实现 EventBus（订阅/发布）
2. 实现 ResourceManager（金币/食物/人口的增删查）
3. 实现 ConfigManager（JSON 加载，使用 nlohmann/json 或手动解析）
4. 实现 ServiceLocator
5. 实现资源变更事件，让 UI 可以监听

**可运行验证：** 通过控制台输出验证事件发布/订阅，验证资源增减逻辑

### Phase 4：世界/地图系统（预计：1天）

**目标：** 创建网格地图和地形系统

**新增类：** `Tile`、`TileMap`、`Camera`

**任务：**
1. 实现 Tile（地形类型、可行走性）
2. 实现 TileMap（二维网格、查询接口）
3. 实现 Camera（平移、缩放）
4. 在 GameScene 中渲染地图（使用纯色矩形代表不同地形）
5. 支持鼠标拖动地图

**可运行验证：** 可以看到彩色网格地图，可以鼠标拖动平移

### Phase 5：实体系统（预计：1.5天）

**目标：** 实现 Entity 体系和建筑系统

**新增类：** `Entity`、`Unit`、`Building` 及子类、`TransformComponent`、`RenderComponent`、`EntityFactory`

**任务：**
1. 实现 Entity 基类（ID、激活状态、虚函数接口）
2. 实现 TransformComponent、RenderComponent
3. 实现 Building 基类及所有建筑子类
4. 实现 Building 的渲染（使用不同颜色的矩形块）
5. 实现建造功能（点击放置建筑）
6. 实现资源产出逻辑（农场产食物，金矿产金币）

**可运行验证：** 可以在地图上放置建筑，建筑产生资源，资源 HUD 实时更新

### Phase 6：单位系统（预计：1.5天）

**目标：** 实现士兵和敌人的基础框架

**新增类：** `Soldier`、`Enemy`、`CombatComponent`、`MovementComponent`、`Pathfinder`

**任务：**
1. 实现 MovementComponent（速度、目标、路径）
2. 实现 CombatComponent（攻击力、防御力、生命值）
3. 实现 A* 寻路 Pathfinder
4. 实现 Soldier（渲染、移动、从兵营招募）
5. 实现 Enemy（渲染、移动、从地图边缘生成）

**可运行验证：** 可以招募士兵，士兵在地图上移动；敌人从边缘生成并走向村庄

### Phase 7：AI 系统（预计：1.5天）

**目标：** 实现单位状态机 AI

**新增类：** `IState`、`StateMachine`、`IdleState`、`PatrolState`、`CombatState`、`FleeState`

**任务：**
1. 实现 FSM 框架
2. 实现 4 个基础状态
3. 实现状态转换条件
4. 将 StateMachine 集成到 Unit
5. 测试 AI 行为：空闲→发现敌人→战斗→胜利→继续巡逻

**可运行验证：** 士兵和敌人拥有自主 AI 行为，不需要玩家手动控制

### Phase 8：战斗系统（预计：1天）

**目标：** 实现战斗结算

**新增类：** 在 CombatComponent 中扩展战斗逻辑

**任务：**
1. 实现近战战斗逻辑（攻击-防御=伤害）
2. 实现攻击间隔/冷却
3. 实现死亡处理（移除实体、发出事件）
4. 实现击杀奖励（金币）

**可运行验证：** 士兵和敌人相遇后自动战斗，直到一方死亡

### Phase 9：敌人波次系统（预计：1天）

**目标：** 实现敌人波次管理

**新增类：** `SpawnManager`

**任务：**
1. 实现波次配置加载
2. 实现 SpawnManager（控制生成时机和数量）
3. 实现波次递增难度
4. 实现波次间隔（玩家准备时间）

**可运行验证：** 敌人分波次进攻，难度逐渐增加

### Phase 10：UI 系统（预计：1.5天）

**目标：** 实现完整的游戏 UI

**新增类：** `UIElement`、`Button`、`Label`、`Panel`、`HUD`、`BuildMenu`、`UIManager`

**任务：**
1. 实现 UIElement 基类
2. 实现 Button、Label、Panel
3. 实现 HUD（资源条、选中信息）
4. 实现 BuildMenu（建筑选择面板）
5. 实现 UI 事件处理（按钮点击建造）

**可运行验证：** 完整的游戏可交互 UI

### Phase 11：胜利/失败条件（预计：0.5天）

**目标：** 实现游戏结束逻辑

**任务：**
1. 指挥部被摧毁 → 游戏失败
2. 存活所有波次 → 游戏胜利
3. 实现结算界面

### Phase 12：动画系统（预计：1天）

**目标：** 添加精灵动画

**新增类：** `Animation`

**任务：**
1. 实现帧动画类
2. 为士兵和敌人添加简单动画（空闲/移动/攻击）
3. 建筑建造动画（可选）

### Phase 13：音频系统（预计：0.5天）

**目标：** 添加音效和背景音乐

**新增类：** `AudioManager`（实现）

**任务：**
1. 实现 AudioManager
2. 添加音效事件触发（战斗、建造、波次开始）
3. 添加背景音乐

### Phase 14：打磨与交付（预计：1天）

**目标：** 完善细节，准备提交

**任务：**
1. 数值平衡调整（通过修改 JSON 配置文件）
2. Bug 修复
3. 代码审查和清理
4. README 文档
5. 最终 Git 提交

### 开发路线总览图

```text
Phase 0   ██ 基础设施 (0.5天)
Phase 1   ██ 核心框架 (1天)
Phase 2   ██ 场景系统 (0.5天)
Phase 3   ██ 资源与事件 (1天)
Phase 4   ██ 地图系统 (1天)
Phase 5   ██ 实体/建筑 (1.5天)
Phase 6   ██ 单位系统 (1.5天)
Phase 7   ██ AI 系统 (1.5天)
Phase 8   ██ 战斗系统 (1天)
Phase 9   ██ 波次系统 (1天)
Phase 10  ██ UI 系统 (1.5天)
Phase 11  ██ 胜负条件 (0.5天)
Phase 12  ██ 动画系统 (1天)
Phase 13  ██ 音频系统 (0.5天)
Phase 14  ██ 打磨交付 (1天)
────────────────────────────
总计：        ~13.5 天
```

---

## 8. 附录：现有项目问题诊断与修正建议

### 8.1 已发现的问题

| 编号 | 问题 | 严重程度 | 修正方案 |
|------|------|---------|---------|
| 1 | `main.cpp` 位于 `assets/` 目录 | **高** | 移至 `src/main.cpp`，源码不应在资源目录 |
| 2 | C++ 标准设置为 C++17 | **中** | 修改 `.vcxproj` 中 `<LanguageStandard>` 为 `stdcpp20` |
| 3 | 缺少 `.gitignore` | **中** | 创建 `.gitignore`，忽略 `x64/`、`.vs/`、`*.user` |
| 4 | 缺少 `include/` 和 `src/` 子目录结构 | **中** | 按模块创建子目录 |
| 5 | 只配置了 Debug x64 | **低** | 同时配置 Release x64（课程设计可暂时只保留 Debug） |
| 6 | 没有 Git 仓库 | **低** | 初始化 Git 仓库 |

### 8.2 立即要做的修正（Phase 0）

在开始任何编码之前，先完成 Phase 0 的基础设施修正。后续所有 Phase 都基于正确的项目结构进行。

---

> **文档结束**
>
> 本文档定义了 Border Commander 项目的完整软件架构设计。
> 所有设计决策均经过权衡分析，在"课程设计优秀作品"这一目标下做出了最佳选择。
> 后续实现阶段应严格遵循本文档的架构约定。
> 如发现架构问题需要调整，应在此文档中更新并评审后再修改代码。
