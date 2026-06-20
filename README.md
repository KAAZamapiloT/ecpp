<p align="center">
  <img src="assets/logo.png" alt="EC++ Logo" width="400"/>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/version-3.0-blue.svg" alt="Version">
  <img src="https://img.shields.io/badge/C++-17-purple.svg" alt="C++17">
  <img src="https://img.shields.io/badge/license-MIT-green.svg" alt="License">
  <img src="https://img.shields.io/badge/build-passing-brightgreen.svg" alt="Build">
</p>

---

**EC++** is a robust, blazingly fast, and feature-rich header-only Entity Component System (ECS) library written in pure C++17. In V3, EC++ transitioned to a **AAA-grade Archetype Storage model**, massively increasing cache performance for complex queries and enabling industry-standard game development.

## 📖 Documentation

Want to dive deep into EC++? Check out our official documentation:

*   **[Getting Started Guide](docs/GettingStarted.md)** - Learn how to initialize the library and create your first entities.
*   **[Architecture Overview](docs/Architecture.md)** - Understand the power of Archetypes and how EC++ manages memory under the hood.

---

## ✨ Key Features

*   **Archetype Storage**: Entities are physically grouped in memory by their exact component signature. Iterate over systems with zero cache-misses!
*   **System Dependency Graph**: Automatically resolves execution order using Topological Sorting. Just define your dependencies, and the ECS updates them correctly.
*   **Reactive Hooks**: Attach `OnComponentAdded` callbacks to instantly react to structural changes.
*   **Dynamic Component Manager**: Add raw byte arrays to entities at runtime, allowing Lua or Python scripts to define components dynamically without recompiling C++.
*   **Header-Only & Cross-Platform**: No dependencies. Just drop the `include/ecpp/` directory into your project on Windows, Linux, macOS, iOS, or Android!

---

## 🚀 Quick Look

Here is a glimpse of how clean and fast EC++ is:

### 1. Archetype System Iteration (Max Performance)
Your `System::Update` loops iterate over densely packed contiguous memory blocks.

```cpp
class PhysicsSystem : public ecpp::System {
public:
    void Update(ecpp::Coordinator& coord, float dt) override {
        auto typePos = coord.GetComponentType<Position>();
        auto typeVel = coord.GetComponentType<Velocity>();

        // Iterate linearly through densely packed contiguous memory
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

### 2. System Dependency Graph
No more manual function calls. Define the graph and update everything at once.

```cpp
auto physicsSys = gCoordinator.RegisterSystem<PhysicsSystem>();
auto renderSys = gCoordinator.RegisterSystem<RenderSystem>();

// Physics must ALWAYS run before Render
gCoordinator.AddSystemDependency<PhysicsSystem, RenderSystem>();

// Automatically resolves the graph and executes Systems
gCoordinator.UpdateSystems(1.0f);
```

### 3. Reactive Hooks
Run code the exact millisecond a component is structurally added.

```cpp
gCoordinator.OnComponentAdded<Name>([](ecpp::Entity e, Name& name) {
    std::cout << "Entity " << e << " was named " << name.name << "\n";
});
```

---

*Explore `examples/v3_main.cpp` for a complete showcase of all advanced features!*
