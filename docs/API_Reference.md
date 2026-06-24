[Home](../README.md) | [Getting Started](GettingStarted.md) | [Architecture](Architecture.md) | [Advanced Features](AdvancedFeatures.md) | **API Reference**

---

# EC++ API Reference

This document outlines the core public interfaces of the EC++ Engine framework. 

## `ecpp::Coordinator`
The central hub that manages entities, components, and systems.

### Entity Management
*   `Entity CreateEntity()`: Spawns a new empty entity and returns its ID.
*   `void DestroyEntity(Entity entity)`: Destroys an entity, removes all its components, and destroys any children in its Hierarchy.
*   `bool IsAlive(Entity entity) const`: Returns true if the entity has not been destroyed.

### Component Management
*   `template<typename T> void RegisterComponent()`: Must be called once before a component type can be used.
*   `template<typename T> void AddComponent(Entity entity, T component)`: Adds a component to an entity. Moves the entity to a new Archetype block.
*   `template<typename T> void RemoveComponent(Entity entity)`: Removes a component from an entity.
*   `template<typename T> T& GetComponent(Entity entity)`: Returns a reference to the component. Asserts if the entity does not have it.
*   `template<typename T> bool HasComponent(Entity entity)`: Returns true if the entity has the specified component type.

### Hierarchy Management (V6)
*   `void AddChild(Entity parent, Entity child)`: Attaches `child` to `parent`. If `child` already has a parent, it is reparented.
*   `void RemoveChild(Entity parent, Entity child)`: Detaches `child` from `parent`.

## `ecpp::System`
The base class for all logic processing.

*   `virtual void Update(Coordinator& coord, float dt)`: The main loop where you write your logic. Iterate over `mArchetypes` to process data.
*   `std::vector<Archetype*> mArchetypes`: The cached, dense memory blocks that match this system's Signature.

## `ecpp::CommandBuffer` (V4)
Thread-safe queue for ECS mutations during parallel processing.

*   `void QueueCreateEntity(std::function<void(Entity)> onCreated)`: Queues entity creation.
*   `void QueueDestroyEntity(Entity entity)`: Queues entity destruction.
*   `void QueueAddComponent(Entity entity, T component)`: Queues a component addition.
*   `void Execute(Coordinator& coord)`: Flushes the buffer sequentially. Must be called on the main thread.

## `ecpp::Serializer` (V6)
Automated JSON Game State generation using Reflection.

*   `template<typename T> void RegisterComponentSerializer(Coordinator& coord)`: Links a reflected component to the serializer.
*   `void SaveToJSON(Coordinator& coord, const std::string& filepath)`: Iterates all living entities and dumps their components to a JSON file.

## `ecpp::SpatialHash` (V5)
High-performance 2D grid for rapid proximity querying and culling.

*   `SpatialHash(float cellSize)`: Initializes the grid with a specified cell size (e.g., 100.0f units).
*   `void UpdateEntity(Entity entity, float x, float y)`: Registers or updates an entity's position in the grid.
*   `std::vector<Entity> QueryAABB(float minX, float minY, float maxX, float maxY) const`: Returns all entities inside the bounding box in `O(1)` time.
