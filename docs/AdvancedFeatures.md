[Home](../README.md) | [Getting Started](GettingStarted.md) | [Architecture](Architecture.md) | **Advanced Features** | [API Reference](API_Reference.md)

---

# Advanced Engine Features (V4-V6)

EC++ comes packed with professional tools designed specifically for Game Engine developers. If you are building a custom engine (similar to Godot or Unity), these tools will solve your hardest architectural problems.

---

## 1. Reflection (Building an Editor UI)

### The Problem:
If you look at Unity or Godot, when you click an object, an "Inspector" window pops up showing all its properties (Health, Speed, Color) with editable sliders. In standard C++, building this requires you to manually write UI code for every single component in your game. 

### The EC++ Solution:
EC++ provides a zero-dependency macro system that automatically parses your structs and exposes their metadata (names, types, and memory offsets) to your engine at runtime.

```cpp
struct Weapon {
    int damage;
    float range;
};

// 1. Tell EC++ to analyze this struct
ECPP_REFLECT_BEGIN(Weapon)
    ECPP_REFLECT_FIELD(Weapon, damage),
    ECPP_REFLECT_FIELD(Weapon, range)
ECPP_REFLECT_END()
```

### The Use Case:
In your engine's UI loop (using something like ImGui), you can now loop over ANY component dynamically. Your engine will automatically know that `damage` is an `int` and `range` is a `float`, allowing you to dynamically draw sliders without ever hardcoding `Weapon` into your UI logic!

---

## 2. JSON Serialization (Save/Load States)

### The Problem:
Implementing a "Save Game" feature usually requires writing massive, brittle functions that manually write every variable to a text file. If you add a new variable to a component, you have to remember to update your Save function, or the save file breaks.

### The EC++ Solution:
Because we built the Reflection system above, EC++ comes with an automated `Serializer`.

```cpp
ecpp::Serializer serializer;
serializer.RegisterComponentSerializer<Weapon>(coord);

// Instantly dumps every living entity and their components into a formatted JSON file!
serializer.SaveToJSON(coord, "savegame.json");
```

### The Use Case:
When a player hits "Quicksave", or when you want to save a Level you just built in your custom Editor, one line of code captures the entire Game State. You never have to manually write save/load boilerplate again.

---

## 3. Entity Hierarchies (Parent/Child Trees)

### The Problem:
ECS architectures are flat. Everything is just data in arrays. But games are hierarchical. 
If a Player entity equips a Sword entity, the Sword needs to move when the Player moves. If the Player is destroyed, the Sword must also be destroyed.

### The EC++ Solution:
EC++ natively supports Linked Tree relationships directly inside the `Coordinator`. It does not use `std::vector` (which causes heap allocations and cache misses). It uses purely contiguous indices.

```cpp
ecpp::Entity player = coord.CreateEntity();
ecpp::Entity sword = coord.CreateEntity();

// The Sword is now physically parented to the Player.
coord.AddChild(player, sword);
```

### The Use Case:
This is exactly how Godot's `SceneTree` or Unity's `Transform` hierarchies work. You can use this to build complex multipart bosses (Torso -> Arm -> Hand -> Gun), or nested UI Canvas panels. If you call `coord.DestroyEntity(player)`, the sword and everything attached to it will automatically be cleaned up.

---

## 4. Command Buffers (Multithreading)

### The Problem:
To get maximum performance, you might want to run your Systems across a Thread Pool (a Job System). 
However, what happens if Thread A decides to `DestroyEntity(1)` while Thread B is currently iterating over Entity 1? Your game crashes with a Segmentation Fault.

### The EC++ Solution:
You pass a `CommandBuffer` into your worker threads. Instead of immediately modifying the ECS, threads "queue" their requests.

```cpp
void WorkerThread(ecpp::Coordinator& coord, ecpp::CommandBuffer& cmdBuf) {
    // We want to destroy this entity, but we queue it up instead of doing it instantly
    cmdBuf.QueueDestroyEntity(entity);
}

// Back on the Main Thread, after all jobs finish safely:
cmdBuf.Execute(coord); 
```

### The Use Case:
If a massive fireball hits 50 enemies simultaneously across 8 different CPU threads, they all queue `QueueDestroyEntity()`. The Command Buffer prevents thread collisions, ensuring your engine remains absolutely stable while heavily parallelized.
