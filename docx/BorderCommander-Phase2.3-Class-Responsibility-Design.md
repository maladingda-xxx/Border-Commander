# Border Commander — Phase 2.3: Class Responsibility Design

> 文档版本：v1.0
> 创建日期：2026-07-03
> 阶段目标：设计每个核心类的职责边界，不编写任何代码
> 技术栈：C++20 / SFML 3.1.0 / Visual Studio 2022
> 前置文档：Phase 2.0 Software Architecture Design, Phase 2.2 Module Design

---

## 目录

1. [设计原则](#1-设计原则)
2. [模块与类映射总览](#2-模块与类映射总览)
3. [Core 模块类设计](#3-core-模块类设计)
   - [Application](#31-application)
   - [Game](#32-game)
   - [Window](#33-window)
4. [Scene 模块类设计](#4-scene-模块类设计)
   - [Scene](#41-scene)
   - [SceneManager](#42-scenemanager)
5. [Entity 模块类设计](#5-entity-模块类设计)
   - [GameObject](#51-gameobject)
   - [Character](#52-character)
   - [Soldier](#53-soldier)
   - [Enemy](#54-enemy)
   - [Building](#55-building)
6. [Component 模块类设计](#6-component-模块类设计)
   - [HealthComponent](#61-healthcomponent)
7. [Inventory 模块类设计](#7-inventory-模块类设计)
   - [Inventory](#71-inventory)
8. [AI 模块类设计](#8-ai-模块类设计)
   - [AIController](#81-aicontroller)
   - [StateMachine](#82-statemachine)
9. [Battle 模块类设计](#9-battle-模块类设计)
   - [BattleSystem](#91-battlesystem)
   - [Weapon](#92-weapon)
10. [Animation 模块类设计](#10-animation-模块类设计)
    - [Animation](#101-animation)
    - [AnimationController](#102-animationcontroller)
11. [Manager 模块类设计](#11-manager-模块类设计)
    - [ResourceManager](#111-resourcemanager)
    - [AudioManager](#112-audiomanager)
    - [InputManager](#113-inputmanager)
12. [World 模块类设计](#12-world-模块类设计)
    - [CollisionManager](#121-collisionmanager)
    - [Camera](#122-camera)
    - [TileMap](#123-tilemap)
13. [UI 模块类设计](#13-ui-模块类设计)
    - [UIManager](#131-uimanager)
    - [Button](#132-button)
    - [Panel](#133-panel)
    - [GameHUD](#134-gamehud)
14. [全局 SOLID 审查](#14-全局-solid-审查)
15. [God Object 预防](#15-god-object-预防)
16. [未来扩展分析](#16-未来扩展分析)
17. [最终审查](#17-最终审查)

---

## 1. 设计原则

### 1.1 核心设计原则

| 原则 | 含义 | 检查标准 |
|------|------|---------|
| **单一职责 (SRP)** | 每个类只有一个改变的理由 | 能用一句话描述类的职责吗？ |
| **开闭原则 (OCP)** | 对扩展开放，对修改关闭 | 新增功能是否需要修改现有类？ |
| **里氏替换 (LSP)** | 子类必须能完全替换父类 | 使用基类引用的代码能在不修改的情况下接受子类吗？ |
| **接口隔离 (ISP)** | 接口应该小而专一 | 是否有类被迫实现它不需要的方法？ |
| **依赖反转 (DIP)** | 依赖抽象而非具体实现 | 高层模块是否依赖低层模块的具体类？ |

### 1.2 架构约束

```
Layer 3 (Assembly):     Core ——→ 所有层
Layer 2 (Orchestration): Scene, UI, Manager ——→ Layer 1, Layer 0
Layer 1 (Domain Logic):  Entity, AI, Battle, World ——→ Layer 0
Layer 0 (Foundation):    Component, Animation, Event, Resource, Utils ——→ 同层或下层
```

**铁律：**
- 禁止反向依赖（Layer 0 不能依赖 Layer 1+）
- 禁止循环依赖（任何环都是设计错误）
- 跨层通信优先通过 EventBus
- 所有拥有关系使用 unique_ptr，非拥有关系使用裸指针/引用

### 1.3 文档中使用的类名映射

用户指定的类名与现有设计文档的对应关系：

| 用户指定类名 | 现有设计对应 | 说明 |
|-------------|-------------|------|
| GameObject | Entity | 游戏对象基类 |
| Character | Unit | 可移动+可战斗的角色基类 |
| AIController | 新概念 | AI 控制器，封装 StateMachine 与 Unit 的交互 |
| BattleSystem | BattleResolver | 战斗系统，集中管理战斗逻辑 |
| ResourceManager | ResourcePool + ResourceCost | 资源管理系统 |
| CollisionManager | 新概念 | 碰撞检测管理器 |
| AnimationController | AnimationManager | 动画播放控制器 |
| GameHUD | HUD | 游戏内抬头显示 |
| Window | 新概念 | SFML 窗口的薄封装 |

---

## 2. 模块与类映射总览

| 模块 | 层 | 包含的类 | 类数量 |
|------|----|---------|--------|
| Core | 3 | Application, Game, Window, GameClock | 4 |
| Scene | 2 | Scene, SceneManager, GameScene, MainMenuScene, PauseScene, GameOverScene | 6 |
| Entity | 1 | GameObject, Character, Soldier, Enemy, Building, Headquarters, Barracks, Farm, GoldMine, Wall, GameObjectManager, GameObjectFactory | 12 |
| Component | 0 | Component, TransformComponent, RenderComponent, MovementComponent, HealthComponent, CombatComponent, ProductionComponent, AIContextComponent | 8 |
| AI | 1 | IState, StateMachine, AIController, IdleState, PatrolState, CombatState, FleeState, MoveToState | 8 |
| Battle | 1 | BattleSystem, Weapon, DamageCalculator, AttackResult | 4 |
| Animation | 0 | AnimationClip, Animation, AnimationController, Frame | 4 |
| UI | 2 | UIElement, Button, Label, Panel, ProgressBar, GameHUD, BuildMenu, UIManager | 8 |
| Manager | 2 | InputManager, AudioManager, ConfigManager, SpawnManager, ServiceLocator | 5 |
| World | 1 | Tile, TileMap, Pathfinder, Camera, CollisionManager | 5 |
| Resource | 0 | ResourceType, ResourceManager, ResourceCost | 3 |
| Event | 0 | Event, EventBus + 6 Event 子类 | 8 |
| Utils | 0 | Random, Logger, Timer, Constants | 4 |

---

## 3. Core 模块类设计

### 3.1 Application

#### Purpose
**为什么存在**——程序入口封装。`main()` 函数中只做一件事：创建 Application 并调用 run()。将 SFML 窗口创建、引擎初始化、游戏启动的细节从全局作用域移入一个明确的类中。

**解决什么问题**——避免 main() 变成几十行的初始化代码。隔离 SFML 的初始化和销毁细节。提供干净的程序生命周期边界。

#### Responsibilities
- 创建 SFML RenderWindow（或通过 Window 类）
- 初始化 Engine（Engine 负责注册所有 Service）
- 创建 Game 实例并启动游戏循环
- 处理程序级别的退出（窗口关闭、Alt+F4等）
- 在退出时按逆序销毁所有子系统
- 捕获并处理程序级别的异常

#### Non-responsibilities
- 不包含任何游戏逻辑
- 不管理游戏内资源
- 不处理游戏输入（那是 InputManager 的职责）
- 不直接访问 Entity、Scene、UI 等游戏概念
- 不管理帧率（那是 Game/GameClock 的职责）
- 不直接持有 SFML 窗口的操作（由 Window 类封装）

#### Expected Member Variables
- `m_window` — 封装的窗口对象（Window 实例）
- `m_engine` — 引擎实例（子系统初始化器）
- `m_game` — 游戏主循环实例
- `m_isRunning` — 程序运行状态标志

#### Expected Public Methods
- `run` — 启动程序（初始化 → 运行游戏循环 → 清理）
- `quit` — 请求退出程序
- `getWindow` — 获取窗口引用（供 Game 渲染使用）

#### Expected Private Methods
- `initialize` — 顺序初始化：创建 Window → 创建 Engine → 创建 Game
- `shutdown` — 逆序销毁：销毁 Game → 销毁 Engine → 销毁 Window
- `handleFatalError` — 致命错误处理（显示错误信息、记录日志、安全退出）

#### Dependencies
- **Depends on**: Window, Engine, Game
- **Used by**: `main()` 函数（仅此一处）
- **Creates**: Window, Engine, Game
- **Communicates with**: Window（创建和销毁）、Engine（初始化和销毁）、Game（启动和控制）

#### Lifetime
- **Owner**: `main()` 函数（栈上的局部变量，或通过 unique_ptr）
- **Created**: `main()` 函数开始时
- **Destroyed**: `main()` 函数返回前
- **Uniqueness**: 整个进程只有一个实例
- **Multiple instances**: 不允许

#### Future Extension
| 扩展 | 影响 | 变更 |
|------|------|------|
| 启动参数解析 | 低 | 在 run() 之前解析命令行参数 |
| 多窗口支持 | 中 | m_window 变成容器 |
| 热重载 | 低 | 在 quit 前添加 reload 逻辑 |

#### Review
- **SRP**: 通过。Application 只负责"启动和停止"，不包含游戏逻辑
- **OCP**: 通过。新增功能（如启动画面）通过扩展 initialize 流程，不修改核心逻辑
- **LSP**: N/A，不参与继承
- **ISP**: N/A，不实现接口
- **DIP**: 通过。依赖 Game 和 Engine 的具体类，但这两个类在 Core 模块内，属于同一层，可以接受

---

### 3.2 Game

#### Purpose
**为什么存在**——游戏主循环的容器。将"循环机制"（何时更新、何时渲染、帧率控制）与"循环内容"（更新什么、渲染什么）分离。Game 只管时钟滴答，SceneManager 决定内容。

**解决什么问题**——避免将游戏循环的计时、帧率管理、事件轮询等机械性代码与游戏逻辑混在一起。提供固定时间步长 + 可变渲染的标准游戏循环。

#### Responsibilities
- 实现固定时间步长的游戏循环
- 管理 GameClock（deltaTime、帧率统计）
- 轮询 SFML 窗口事件并转发给 InputManager
- 每帧调用 SceneManager 的 update(dt) 和 render(window)
- 控制游戏循环的启动和停止
- 提供 FPS 统计和性能数据

#### Non-responsibilities
- 不知道 Entity、Building、Soldier 等游戏概念的存在
- 不直接处理输入（通过 SceneManager 转发）
- 不渲染任何东西（通过 SceneManager 调用）
- 不管理场景切换（那是 SceneManager 的职责）
- 不初始化子系统（那是 Engine 的职责）
- 不创建窗口（那是 Application 的职责）

#### Expected Member Variables
- `m_sceneManager` — 场景管理器引用
- `m_clock` — 游戏时钟实例
- `m_isRunning` — 游戏循环运行状态
- `m_fpsCounter` — FPS 统计器

#### Expected Public Methods
- `run` — 进入主循环（循环直到退出）
- `quit` — 请求退出游戏循环
- `getDeltaTime` — 获取当前帧的时间步长
- `getFPS` — 获取当前帧率

#### Expected Private Methods
- `update` — 单帧逻辑更新（固定步长）
- `render` — 单帧渲染（可变帧率，含插值）
- `processEvents` — 轮询窗口事件并分发
- `processOneFrame` — 完整的一帧：事件 → N次固定更新 → 渲染

#### Dependencies
- **Depends on**: SceneManager, GameClock, Window（通过 SFML 的 RenderWindow 引用）
- **Used by**: Application
- **Creates**: 无（GameClock 是成员）
- **Communicates with**: SceneManager（更新/渲染调度）、Window（事件轮询、渲染目标）

#### Lifetime
- **Owner**: Application
- **Created**: Application::initialize() 中，Engine 初始化之后
- **Destroyed**: Application::shutdown() 中，先于 Engine 销毁
- **Uniqueness**: 唯一实例
- **Multiple instances**: 不允许

#### Future Extension
| 扩展 | 影响 | 变更 |
|------|------|------|
| 多线程渲染 | 高 | 需要将 update 和 render 分离到不同线程 |
| 录制/回放 | 中 | 在 update 中注入录制/回放钩子 |
| 帧率解锁 | 低 | 修改 GameClock 的配置 |

#### Review
- **SRP**: 通过。Game 只负责"游戏循环机制"
- **OCP**: 通过。新增功能通过组合（添加新成员）而非修改现有代码
- **LSP**: N/A
- **ISP**: N/A
- **DIP**: 通过。Game 依赖 SceneManager（同层 Orchestration），不依赖具体场景

---

### 3.3 Window

#### Purpose
**为什么存在**——对 SFML RenderWindow 的薄封装。隔离第三方库依赖，提供游戏所需的窗口操作接口。如果将来更换渲染库，只需修改 Window 类。

**解决什么问题**——避免整个项目中到处直接使用 `sf::RenderWindow`。提供统一的窗口尺寸管理、全屏切换、标题设置接口。

#### Responsibilities
- 封装 SFML RenderWindow 的创建和销毁
- 管理窗口尺寸、标题、全屏状态
- 提供渲染目标引用（供 Game 和 Scene 渲染）
- 提供窗口事件轮询接口
- 管理窗口图标和光标
- 处理窗口大小调整事件

#### Non-responsibilities
- 不渲染任何游戏内容
- 不处理游戏逻辑
- 不管理游戏内 UI
- 不处理输入映射（那是 InputManager 的职责）

#### Expected Member Variables
- `m_renderWindow` — SFML 窗口实例（或通过 unique_ptr 持有）
- `m_windowTitle` — 窗口标题字符串
- `m_width` / `m_height` — 窗口尺寸
- `m_isFullscreen` — 全屏状态标志
- `m_videoModes` — 可用的显示模式列表

#### Expected Public Methods
- `create` — 创建窗口（传入标题、尺寸、样式）
- `close` — 关闭窗口
- `isOpen` — 检查窗口是否打开
- `pollEvent` — 轮询单个窗口事件
- `clear` — 清空渲染目标
- `display` — 交换缓冲区
- `getRenderTarget` — 获取底层渲染目标引用
- `setTitle` — 设置窗口标题
- `getSize` — 获取窗口尺寸
- `setFullscreen` — 切换全屏模式
- `setFramerateLimit` — 设置帧率上限

#### Expected Private Methods
- `initializeSFML` — SFML 库级别的初始化
- `shutdownSFML` — SFML 库级别的清理

#### Dependencies
- **Depends on**: SFML（Window、Graphics 模块）
- **Used by**: Application, Game（渲染时使用）
- **Creates**: sf::RenderWindow
- **Communicates with**: Game（提供事件轮询和渲染目标）

#### Lifetime
- **Owner**: Application
- **Created**: Application::initialize() 的第一步
- **Destroyed**: Application::shutdown() 的最后一步
- **Uniqueness**: 唯一（MVP 阶段单窗口）
- **Multiple instances**: 未来可能（多显示器）

#### Future Extension
| 扩展 | 影响 | 变更 |
|------|------|------|
| 多窗口/多显示器 | 高 | m_renderWindow 变成容器 |
| 替换渲染库 | 中 | 修改内部实现，保持接口不变 |
| 窗口状态保存/恢复 | 低 | 添加序列化接口 |

#### Review
- **SRP**: 通过。只负责窗口抽象
- **OCP**: 通过。可扩展（如添加窗口回调）而不修改现有接口
- **LSP**: N/A
- **ISP**: N/A
- **DIP**: 通过。隔离了 SFML 依赖

---

## 4. Scene 模块类设计

### 4.1 Scene

#### Purpose
**为什么存在**——定义场景生命周期的抽象接口。游戏中的每个"画面"（主菜单、游戏中、暂停、结算）都是不同的场景，但它们共享相同的生命周期结构。

**解决什么问题**——让 SceneManager 可以用统一的方式管理不同类型的场景。新增场景（如设置画面、加载画面）不需要修改 SceneManager。

#### Responsibilities
- 定义场景生命周期接口：onEnter → (update → render → handleInput)* → onExit
- 提供场景标识（场景ID、激活状态）
- 为所有具体场景定义多态基类

#### Non-responsibilities
- 不包含任何具体场景的实现
- 不管理场景切换（那是 SceneManager 的职责）
- 不持有子系统引用（由具体子类决定需要什么）
- 不加载资源（由具体子类负责）

#### Expected Member Variables
- `m_sceneId` — 场景唯一标识
- `m_isActive` — 场景是否处于激活状态

#### Expected Public Methods
- `onEnter` — 纯虚。场景进入时调用（初始化逻辑）
- `onExit` — 纯虚。场景退出时调用（清理逻辑）
- `update` — 纯虚。每帧逻辑更新
- `render` — 纯虚。每帧渲染
- `handleInput` — 纯虚。处理游戏命令
- `isActive` — 查询是否激活
- `getSceneId` — 获取场景ID

#### Expected Private Methods
- 无（抽象基类，不包含私有实现）

#### Dependencies
- **Depends on**: 无（纯抽象接口）
- **Used by**: SceneManager
- **Creates**: 无
- **Communicates with**: 由子类决定

#### Lifetime
- **Owner**: SceneManager（通过 unique_ptr）
- **Created**: SceneManager::pushScene() 或 switchTo() 时
- **Destroyed**: SceneManager::popScene() 或 switchTo() 替换时
- **Uniqueness**: 可以有多个实例（不同的具体场景）
- **Multiple instances**: 是（MainMenuScene, GameScene, PauseScene, GameOverScene 同时存在多个）

#### Future Extension
| 扩展 | 影响 | 变更 |
|------|------|------|
| 场景过渡动画 | 中 | 在 SceneManager 的切换流程中增加过渡 |
| 场景参数传递 | 低 | 在 onEnter 中接收参数结构体 |
| 异步场景加载 | 中 | 添加 onLoadAsync 虚函数 |

#### Review
- **SRP**: 通过。只定义场景抽象接口
- **OCP**: 通过。新增场景类型 = 新增子类，不修改基类
- **LSP**: 通过。所有子类必须实现完整的生命周期接口
- **ISP**: 通过。接口只包含场景必需的方法
- **DIP**: 通过。SceneManager 依赖抽象 Scene 而非具体场景

---

### 4.2 SceneManager

#### Purpose
**为什么存在**——管理场景栈和场景切换。支持 push/pop 场景操作（暂停画面覆盖在游戏画面之上），而非简单的场景替换。

**解决什么问题**——将场景的生命周期管理、切换逻辑、栈操作集中在一个地方。避免 Game 类中堆砌场景切换代码。

#### Responsibilities
- 管理场景栈（scene stack）
- 支持 pushScene（叠加场景）、popScene（移除顶层）、switchTo（替换所有）
- 每帧调用栈顶场景的 update/render
- 将输入分发给栈顶场景（或从顶向下传递直到被处理）
- 处理延迟的场景操作（当帧中间请求切换时，在帧末执行）
- 保证场景生命周期调用顺序正确

#### Non-responsibilities
- 不包含任何具体场景的逻辑
- 不管理实体、UI、资源
- 不渲染任何东西（调用场景的 render）

#### Expected Member Variables
- `m_sceneStack` — 场景栈（底部 → 顶部）
- `m_pendingOperations` — 待处理的场景操作队列

#### Expected Public Methods
- `pushScene` — 在栈顶推入新场景（覆盖当前场景）
- `popScene` — 移除栈顶场景
- `switchTo` — 清空栈并切换至新场景
- `update` — 调用栈顶场景的 update
- `render` — 从栈底到栈顶依次渲染（或只渲染栈顶，取决于配置）
- `handleInput` — 分发输入给场景
- `getCurrentScene` — 获取当前活跃的栈顶场景
- `isEmpty` — 栈是否为空

#### Expected Private Methods
- `processPendingOperations` — 在帧末执行待处理的 push/pop/switch 操作
- `validateOperation` — 验证场景操作是否合法（防止 pop 空栈等）

#### Dependencies
- **Depends on**: Scene（抽象基类）
- **Used by**: Game
- **Creates**: 无（接收外部创建的 Scene 实例，管理其生命周期）
- **Communicates with**: Scene（调用生命周期方法）

#### Lifetime
- **Owner**: Game
- **Created**: Game 构造函数中
- **Destroyed**: Game 析构时
- **Uniqueness**: 唯一
- **Multiple instances**: 不允许

#### Future Extension
| 扩展 | 影响 | 变更 |
|------|------|------|
| 场景过渡效果 | 中 | 添加过渡状态机 |
| 场景预加载 | 中 | 添加预加载队列 |
| 场景历史记录 | 低 | 在栈中保留历史场景的元数据 |

#### Review
- **SRP**: 通过。只负责场景栈管理
- **OCP**: 通过。新增场景类型不影响 SceneManager
- **LSP**: N/A
- **ISP**: N/A
- **DIP**: 通过。依赖抽象 Scene 接口

---

## 5. Entity 模块类设计

### 5.1 GameObject

#### Purpose
**为什么存在**——所有游戏对象的统一基类。提供唯一ID、激活状态、组件容器的管理。使 EntityManager 可以用同一种方式遍历和处理所有游戏对象。

**解决什么问题**——避免为每种游戏对象类型编写单独的管理代码。提供统一的组件查询和生命周期接口。

#### Responsibilities
- 提供全局唯一的实体ID
- 管理激活/非激活状态
- 持有组件容器（Component 的映射表）
- 提供类型安全的组件查询接口
- 定义虚函数 update 和 render（子类可选重写）
- 提供标签系统用于分类查询

#### Non-responsibilities
- 不包含任何具体游戏逻辑（战斗、移动、生产等，由组件和 System 负责）
- 不加载图片/资源
- 不播放音效
- 不知道 AI 的存在
- 不知道 UI 的存在
- 不直接访问 EventBus（子类可以，但基类不强制要求）
- 不拥有 StateMachine

#### Expected Member Variables
- `m_id` — 全局唯一标识符（递增分配）
- `m_isActive` — 激活状态标志
- `m_tag` — 分类标签（"Soldier", "Enemy", "Building" 等）
- `m_components` — 组件映射表（ComponentType → unique_ptr<Component>）

#### Expected Public Methods
- `getId` — 获取实体ID
- `isActive` — 检查是否激活
- `setActive` — 设置激活状态
- `getComponent<T>` — 按类型获取组件（模板方法，返回 T*）
- `addComponent<T>` — 添加组件（模板方法）
- `hasComponent<T>` — 检查是否拥有某类型组件
- `removeComponent<T>` — 移除组件
- `getTag` / `setTag` — 标签的读写
- `update` — 虚函数，子类可重写添加逻辑
- `render` — 虚函数，子类可重写添加渲染

#### Expected Private Methods
- `generateNextId` — 静态方法，生成下一个唯一ID

#### Dependencies
- **Depends on**: Component（基类）
- **Used by**: GameObjectManager, Character, Building, 以及所有需要遍历实体的系统
- **Creates**: Component 子类实例（通过 addComponent 模板方法）
- **Communicates with**: Component（存储和查询）

#### Lifetime
- **Owner**: GameObjectManager（通过 unique_ptr）
- **Created**: GameObjectFactory::create 方法中
- **Destroyed**: 被 GameObjectManager::remove 标记，下一帧 processPending 中销毁
- **Uniqueness**: 每个游戏对象一个实例
- **Multiple instances**: 是（地图上所有单位、建筑、投射物等）

#### Future Extension
| 扩展 | 影响 | 变更 |
|------|------|------|
| 实体序列化 | 中 | 添加 serialize/deserialize 虚方法 |
| 实体池复用 | 中 | 添加 reset 方法和对象池支持 |
| 层级关系（父子实体） | 中 | 添加 parent/children 引用 |
| LOD 支持 | 低 | 添加 lodLevel 成员 |

#### Review
- **SRP**: 通过。只负责组件管理和身份标识
- **OCP**: 通过。新增功能通过添加新 Component 类型，不修改 GameObject
- **LSP**: 通过。子类（Character, Building）是真正的 is-a 关系
- **ISP**: N/A（只有一个基类，没有实现多个接口）
- **DIP**: 通过。依赖 Component 基类（Layer 1 → Layer 0 的合规依赖）

---

### 5.2 Character

#### Purpose
**为什么存在**——可移动、可战斗的游戏对象基类。Soldier 和 Enemy 的公共父类，封装移动、战斗、生命值等行为的接口调用。

**解决什么问题**——消除 Soldier 和 Enemy 中的重复代码。提供统一的"可移动+可战斗"行为接口。将来新增兵种或角色类型时可复用这些接口。

#### Responsibilities
- 提供移动接口（moveTo、stopMoving、getPosition）
- 提供战斗接口（attack、takeDamage、heal）
- 提供生命状态查询（isAlive、getHealthPercent）
- 提供阵营信息（getTeam、setTeam）
- 将方法调用委托给对应的 Component（MovementComponent、CombatComponent、HealthComponent）

#### Non-responsibilities
- 不执行战斗伤害计算（委托给 BattleSystem）
- 不执行寻路计算（委托给 Pathfinder）
- 不管理自己的 AI 状态（AIController 在外部）
- 不加载资源
- 不渲染（通过 RenderComponent 和 GameObject::render）

#### Expected Member Variables
- `m_team` — 阵营枚举（Player, Enemy, Neutral, Ally）
- （其他数据通过 Component 存储：MovementComponent、CombatComponent、HealthComponent、AIContextComponent）

#### Expected Public Methods
- `moveTo` — 设置移动目标（更新 MovementComponent 的 targetPosition）
- `stopMoving` — 停止移动
- `getPosition` — 获取当前位置（通过 TransformComponent）
- `attack` — 对目标执行攻击（委托给 BattleSystem）
- `takeDamage` — 受到伤害（更新 HealthComponent）
- `heal` — 恢复生命值
- `isAlive` — 检查是否存活
- `getHealthPercent` — 获取生命值百分比
- `getTeam` / `setTeam` — 阵营管理
- `isHostileTo` — 判断与另一个角色是否为敌对关系

#### Expected Private Methods
- `ensureComponent` — 确保必需的 Component 已添加（初始化时调用）

#### Dependencies
- **Depends on**: GameObject, Component (MovementComponent, CombatComponent, HealthComponent, TransformComponent, AIContextComponent)
- **Used by**: AIController, StateMachine, BattleSystem
- **Creates**: 无（组件由 Factories 在创建时添加）
- **Communicates with**: BattleSystem（通过委托）、Pathfinder（通过 MovementSystem）

#### Lifetime
- **Owner**: GameObjectManager（通过 unique_ptr）
- **Created**: GameObjectFactory::createSoldier / createEnemy
- **Destroyed**: 死亡后由 BattleSystem 触发 EntityDestroyedEvent，GameObjectManager 响应销毁
- **Uniqueness**: 每个角色一个实例
- **Multiple instances**: 是

#### Future Extension
| 扩展 | 影响 | 变更 |
|------|------|------|
| 飞行单位 | 中 | 添加 FlyingCharacter 子类，覆盖移动逻辑 |
| 英雄单位 | 中 | 添加 HeroCharacter 子类，添加技能接口 |
| 状态效果（buff/debuff） | 中 | 添加 StatusEffectComponent |
| 经验/等级系统 | 低 | 添加 ExperienceComponent |

#### Review
- **SRP**: 通过。只负责"可移动+可战斗"的接口封装
- **OCP**: 通过。新增角色类型 = 新增子类，基类不需要修改
- **LSP**: 通过。Character 可以替换 GameObject 使用（持有所有必需组件）
- **ISP**: N/A
- **DIP**: 通过。Character 依赖抽象的 Component 数据，不依赖具体系统

---

### 5.3 Soldier

#### Purpose
**为什么存在**——玩家阵营的士兵单位。与 Enemy 区分阵营和行为配置。

**解决什么问题**——承载玩家特定的行为数据（巡逻路径、招募成本）。让玩家可知"这是我的单位"并与其他阵营区分。

#### Responsibilities
- 设置和存储巡逻路径
- 提供招募成本信息
- 继承 Character 的所有移动和战斗能力
- 默认阵营为 Player

#### Non-responsibilities
- 不实现 AI 决策（由 AIController 和 StateMachine 负责）
- 不执行建造
- 不产出资源

#### Expected Member Variables
- `m_patrolPath` — 巡逻路径点列表
- `m_recruitCost` — 招募此士兵的资源消耗

#### Expected Public Methods
- `setPatrolPath` — 设置巡逻路径（同时更新 AIContextComponent）
- `getPatrolPath` — 获取巡逻路径
- `getRecruitCost` — 获取招募成本

#### Expected Private Methods
- 无额外私有方法

#### Dependencies
- **Depends on**: Character
- **Used by**: GameObjectFactory, GameScene（初始化巡逻）, BuildMenu（显示招募信息）
- **Creates**: 无
- **Communicates with**: AIController（通过 AIContextComponent 传递巡逻路径）

#### Lifetime
- **Owner**: GameObjectManager
- **Created**: GameObjectFactory::createSoldier
- **Destroyed**: 死亡时销毁（由 BattleSystem 触发）
- **Uniqueness**: 每个士兵一个实例
- **Multiple instances**: 是（玩家可以招募多个士兵）

#### Future Extension
| 扩展 | 影响 | 变更 |
|------|------|------|
| 多兵种（Archer, Knight） | 低 | 通过配置和组件差异创建，不需要新子类 |
| 兵种升级 | 低 | 替换 CombatComponent 中的 Weapon 实例 |
| 特殊技能 | 中 | 添加 SkillComponent |

#### Review
- **SRP**: 通过。只添加了巡逻路径和招募成本两个额外职责
- **OCP**: 通过。新增兵种通过配置驱动，不需要修改 Soldier 类
- **LSP**: 通过。Soldier 完全替换 Character
- **ISP**: N/A
- **DIP**: 通过

---

### 5.4 Enemy

#### Purpose
**为什么存在**——敌人阵营的单位。与 Soldier 区分阵营、AI 行为和奖励。

**解决什么问题**——承载敌人特有的数据（所属波次、击杀奖励）。让系统区分"敌人"和"友方"。

#### Responsibilities
- 存储所属波次编号
- 存储击杀奖励金币数
- 继承 Character 的所有移动和战斗能力
- 默认阵营为 Enemy

#### Non-responsibilities
- 不实现敌人生成逻辑（由 SpawnManager 负责）
- 不实现 AI 决策

#### Expected Member Variables
- `m_waveNumber` — 所属敌人波次
- `m_bounty` — 击杀后奖励的金币数量

#### Expected Public Methods
- `getWaveNumber` — 获取波次编号
- `getBounty` — 获取击杀奖励

#### Expected Private Methods
- 无额外私有方法

#### Dependencies
- **Depends on**: Character
- **Used by**: SpawnManager, BattleSystem, GameHUD（显示波次信息）
- **Creates**: 无
- **Communicates with**: AIController

#### Lifetime
- **Owner**: GameObjectManager
- **Created**: SpawnManager → GameObjectFactory::createEnemy
- **Destroyed**: 被击杀时销毁；或波次结束后清理
- **Uniqueness**: 每个敌人一个实例
- **Multiple instances**: 是（波次中可能有多个敌人）

#### Future Extension
| 扩展 | 影响 | 变更 |
|------|------|------|
| 多种敌人类型 | 低 | 通过配置驱动，EnemyConfig 包含所有属性差异 |
| Boss 敌人 | 中 | 添加 Boss 子类，额外 Boss 行为 |
| 飞行敌人 | 中 | 需要路径忽略地形 |
| 敌人掉落物品 | 低 | 添加 lootTable 成员 |

#### Review
- **SRP**: 通过。职责明确：敌人身份 + 波次信息 + 奖励
- **OCP**: 通过。敌人类型通过配置驱动，Enemy 类不需要修改
- **LSP**: 通过。Enemy 完全替换 Character
- **ISP**: N/A
- **DIP**: 通过

---

### 5.5 Building

#### Purpose
**为什么存在**——不可移动的游戏对象基类。区别于 Character：不能移动、有建造进度、占地大小。所有建筑类型的公共父类。

**解决什么问题**——统一管理建筑的建造进度、占地格子、完成事件。避免在每种建筑子类中重复建造逻辑。

#### Responsibilities
- 管理建造进度（buildProgress / buildTime）
- 管理占地格子（一个建筑可能占 1x1、2x2 或更大）
- 提供建造完成事件钩子
- 提供被占领的格子列表
- 管理建筑的生命值（通过 HealthComponent）

#### Non-responsibilities
- 不能移动（没有 MovementComponent）
- 不执行攻击（Wall 是被动障碍物，不主动攻击）
- 不自己执行资源产出（由 GameScene 的生产更新循环读取 ProductionComponent）

#### Expected Member Variables
- `m_buildProgress` — 当前建造进度（0.0 ~ 1.0）
- `m_buildTime` — 建造总时间（秒）
- `m_tileSize` — 占地尺寸（宽 × 高，格子数）
- `m_occupiedTiles` — 占用的格子坐标列表

#### Expected Public Methods
- `isBuilt` — 建造是否完成
- `getBuildProgress` — 获取建造进度
- `addBuildProgress` — 增加建造进度（由 GameScene::update 调用）
- `getOccupiedTiles` — 获取占用的格子列表
- `getCost` — 获取建造成本
- `onBuildComplete` — 虚函数，建造完成时的回调（子类重写）

#### Expected Private Methods
- `calculateOccupiedTiles` — 根据位置和 tileSize 计算占用的所有格子

#### Dependencies
- **Depends on**: GameObject, Component（HealthComponent, ProductionComponent, RenderComponent, CollisionComponent）
- **Used by**: GameScene, GameObjectManager, TileMap（更新格子占用状态）
- **Creates**: 无
- **Communicates with**: TileMap（标记格子占用）、EventBus（BuildingPlacedEvent, BuildingReadyEvent）

#### Lifetime
- **Owner**: GameObjectManager
- **Created**: GameObjectFactory::createBuilding
- **Destroyed**: 被敌人摧毁时销毁
- **Uniqueness**: 每个建筑一个实例
- **Multiple instances**: 是（多个农场、兵营等）

#### Future Extension
| 扩展 | 影响 | 变更 |
|------|------|------|
| 建筑升级 | 中 | 添加 upgradeLevel 和升级逻辑 |
| 可维修建筑 | 低 | 添加 repair 方法 |
| 建筑占地旋转 | 低 | 添加 rotation 参数 |
| 科技建筑 | 中 | 新增 TechnologyBuilding 子类 |

#### Review
- **SRP**: 通过。只负责"不可移动 + 建造进度"的建筑公共逻辑
- **OCP**: 通过。新增建筑类型 = 新增子类，Building 基类不需修改
- **LSP**: 通过
- **ISP**: N/A
- **DIP**: 通过

---
