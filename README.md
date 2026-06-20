<p align="center">
  <img src="assets/logo.png" alt="EC++ Logo" width="400"/>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/version-4.0-blue.svg" alt="Version">
  <img src="https://img.shields.io/badge/C++-17-purple.svg" alt="C++17">
  <img src="https://img.shields.io/badge/license-MIT-green.svg" alt="License">
  <img src="https://img.shields.io/badge/build-passing-brightgreen.svg" alt="Build">
</p>

---

**EC++** is a blazingly fast, AAA-grade header-only Entity Component System (ECS) library written in pure C++17. More than just an ECS, it provides professional data-management tools for Game Engine development.

## 📖 Documentation

Want to dive deep into EC++? Check out our official documentation:

*   **[Getting Started Guide](docs/GettingStarted.md)** - Learn how to initialize the library and create your first entities.
*   **[Architecture Overview](docs/Architecture.md)** - Understand the power of Archetypes and how EC++ manages memory under the hood.

---

## ✨ Key Features

*   **Archetype Storage**: Iterate over systems with zero cache-misses! Entities are perfectly packed in memory.
*   **System Dependency Graph**: Automatically resolves execution order using Topological Sorting.
*   **Command Buffers (V4)**: Safely queue ECS mutations (Add/Remove Component, Create/Destroy Entity) across multiple threads during JobSystem execution.
*   **Custom Memory Allocators (V4)**: Prevent heap fragmentation using built-in `LinearAllocator` and `PoolAllocator`.
*   **Reflection & Editor Support (V4)**: Automatically expose C++ component data to your engine's UI Editor (e.g., ImGui) using our zero-dependency macro system.
*   **Header-Only & Cross-Platform**: No dependencies. Just drop the `include/ecpp/` directory into your project!

---

## 🕹️ How to Integrate into Your Game Engine

EC++ is intentionally designed **not** to take over your game loop or window management. It acts as a pure, high-performance data-management layer that seamlessly plugs into your existing architecture.

1. **The Game Loop**: Simply call `gCoordinator.UpdateSystems(dt);` inside your engine's existing Fixed Update or variable Update loop. EC++ doesn't force a specific tick rate on you.
2. **The Editor UI**: Read the `ecpp::ReflectionInfo<T>::GetFields()` arrays when rendering your ImGui or Qt Inspector windows. This allows your engine to automatically draw sliders and checkboxes without hardcoding UI for every component!
3. **The Physics/Render Threads**: If your engine uses a custom thread pool for Physics or Render passes, pass an `ecpp::CommandBuffer` into those worker threads. They can freely queue Entity destructions and creations, and your engine can safely call `cmdBuf.Execute()` back on the main thread when the threads sync.

---

## 🚀 Quick Look

Here is a glimpse of the powerful tools available in V4:

### Thread-Safe Command Buffers
When using a Job System across multiple threads, you cannot safely destroy entities. Use `CommandBuffer` to queue changes and execute them safely later.

```cpp
void DamageSystem::Update(ecpp::Coordinator& coord, ecpp::CommandBuffer& cmdBuf) {
    // ... inside a worker thread
    if (health.current <= 0) {
        cmdBuf.QueueDestroyEntity(entity);
    } else {
        cmdBuf.QueueRemoveComponent<Damage>(entity);
    }
}

// ... back on the main thread after jobs finish
cmdBuf.Execute(coord);
```

### C++ Reflection for Editor UIs
Want to build an Inspector UI for your engine? Just reflect your components.

```cpp
ECPP_REFLECT_BEGIN(Position)
    ECPP_REFLECT_FIELD(Position, x),
    ECPP_REFLECT_FIELD(Position, y)
ECPP_REFLECT_END()

// Later, automatically draw ImGui sliders dynamically:
for (const auto& field : ecpp::ReflectionInfo<Position>::GetFields()) {
    std::cout << field.name << " is of type " << field.typeName << "\n";
}
```

---

## 🛠️ Building & Running Examples

Since EC++ is a header-only library, you don't need to build the library itself. However, you can easily compile and run the provided examples using any modern C++17 compiler (like `g++`, `clang++`, or MSVC).

To run the **V4 Advanced Features Example** from your terminal:

```bash
# Compile the example
g++ -std=c++17 examples/v4_main.cpp -o v4_example

# Run the compiled executable
./v4_example
```

### Included Examples

*   **`examples/v4_main.cpp`** - A comprehensive showcase including Multithreaded Command Buffers, Pool Allocators, and the Reflection system.
*   **`examples/spaceship_benchmark.cpp`** - A classic performance benchmark simulating thousands of spaceships iterating through boundary and movement systems at blazing speeds.
*   **`examples/editor_ui_mockup.cpp`** - Demonstrates how to use the EC++ Reflection system to build a dynamic "ImGui-style" Inspector window for any component.
