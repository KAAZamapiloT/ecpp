# Getting Started with EC++

Welcome to EC++! If you are building a game engine or a highly-optimized game, you've come to the right place. EC++ is a purely data-driven Entity Component System (ECS). 

## What is an ECS and Why Use It?

In traditional Object-Oriented Programming (OOP) engines like early Unity or Godot, you often use Inheritance (`class Player : public GameObject`). As games grow, this becomes a tangled mess (the "Diamond Problem"). Furthermore, OOP scatters data randomly across the heap, causing massive performance drops due to CPU Cache Misses.

**ECS solves this by separating logic from data:**
*   **Entities**: Just an ID (like a social security number). It has no logic.
*   **Components**: Pure data structs (e.g., `Position { x, y }`). They have no functions.
*   **Systems**: Pure logic functions (e.g., `MovementSystem`). They contain no state.

## 1. Installation

EC++ is entirely header-only. This means there is no complex CMake setup or DLLs to link.

1. Download the `include/ecpp/` folder.
2. Drag it into your project's include path.
3. `#include <ecpp/Coordinator.hpp>`

## 2. Your First Engine Loop

The `Coordinator` is the heart of the engine. It manages all your entities, components, and systems.

```cpp
#include <ecpp/Coordinator.hpp>

int main() {
    ecpp::Coordinator coord;
    coord.Init();

    // Your game loop runs here
    bool isRunning = true;
    while (isRunning) {
        float deltaTime = 0.016f; // Assume 60fps
        coord.UpdateSystems(deltaTime);
    }
    
    return 0;
}
```

## 3. Defining Data (Components)

Think of Components as simple "tags" or "backpacks" you can give to an entity. Let's say we are building a 2D Platformer. We need to know where things are, and how fast they are moving.

```cpp
struct Position {
    float x;
    float y;
};

struct Velocity {
    float dx;
    float dy;
};

// Register them before using them!
coord.RegisterComponent<Position>();
coord.RegisterComponent<Velocity>();
```

## 4. Writing Logic (Systems)

A System's only job is to iterate over entities that have a specific set of components. 

*Use Case*: We want a system that moves any entity that has both a `Position` and a `Velocity`. It shouldn't care if the entity is a Player, an Enemy, or a Cloud. If it moves, it processes it.

```cpp
class PhysicsSystem : public ecpp::System {
public:
    void Update(ecpp::Coordinator& coord, float dt) override {
        // We get the internal IDs for the components we care about
        auto typePos = coord.GetComponentType<Position>();
        auto typeVel = coord.GetComponentType<Velocity>();

        // Iterate through perfectly packed memory (Archetypes)
        for (auto* arch : mArchetypes) {
            for (size_t row = 0; row < arch->GetEntityCount(); ++row) {
                // Grab the raw data and modify it
                Position& pos = *(Position*)arch->GetComponentPtr(typePos, row);
                Velocity& vel = *(Velocity*)arch->GetComponentPtr(typeVel, row);

                pos.x += vel.dx * dt;
                pos.y += vel.dy * dt;
            }
        }
    }
};
```

Register the system and tell it what components it cares about (its Signature):
```cpp
auto physicsSys = coord.RegisterSystem<PhysicsSystem>();

ecpp::Signature signature;
signature.set(coord.GetComponentType<Position>());
signature.set(coord.GetComponentType<Velocity>());
coord.SetSystemSignature<PhysicsSystem>(signature);
```

## 5. Spawning the Player

Now, let's spawn an entity and give it our components.

```cpp
ecpp::Entity player = coord.CreateEntity();

// Give the player a starting position and speed
coord.AddComponent(player, Position{100.0f, 200.0f});
coord.AddComponent(player, Velocity{5.0f, 0.0f});
```

That's it! When your `while(isRunning)` loop calls `coord.UpdateSystems()`, your `PhysicsSystem` will automatically find the player and move it to the right. If you spawn 10,000 bullets with the same components, the `PhysicsSystem` will move all of them simultaneously at blazing speeds!
