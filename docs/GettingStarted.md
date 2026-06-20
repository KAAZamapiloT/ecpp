# Getting Started with EC++

## Installation
EC++ is a header-only library. To use it, simply copy the `include/ecpp/` directory into your project's include path.

```bash
cp -r include/ecpp /path/to/your/project/include/
```

Then, include the coordinator in your source files:
```cpp
#include <ecpp/Coordinator.hpp>
```

## Initialization
Before using the ECS, you must initialize the Coordinator and register your components.

```cpp
ecpp::Coordinator gCoordinator;

struct Position { float x, y; };
struct Velocity { float dx, dy; };

int main() {
    gCoordinator.Init();

    gCoordinator.RegisterComponent<Position>();
    gCoordinator.RegisterComponent<Velocity>();
    
    // Create an entity
    ecpp::Entity player = gCoordinator.CreateEntity();
    gCoordinator.AddComponent(player, Position{0.0f, 0.0f});
    
    return 0;
}
```

## Creating Systems
Systems contain the logic that operates on entities. In V3, systems iterate over Archetypes for maximum performance.

```cpp
class MovementSystem : public ecpp::System {
public:
    void Update(ecpp::Coordinator& coord, float dt) override {
        auto typePos = coord.GetComponentType<Position>();
        auto typeVel = coord.GetComponentType<Velocity>();

        for (ecpp::Archetype* arch : mArchetypes) {
            for (size_t row = 0; row < arch->GetEntityCount(); ++row) {
                Position& pos = *(Position*)arch->GetComponentPtr(typePos, row);
                Velocity& vel = *(Velocity*)arch->GetComponentPtr(typeVel, row);
                
                pos.x += vel.dx * dt;
                pos.y += vel.dy * dt;
            }
        }
    }
};
```
