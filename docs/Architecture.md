# EC++ Architecture

EC++ V3 uses a strict **Archetype Storage** model.

## What is an Archetype?
An archetype represents a specific combination of components. For example, an entity with just a `Transform` belongs to Archetype A. An entity with a `Transform` and a `RigidBody` belongs to Archetype B.

When you add or remove a component from an entity, it undergoes a **Structural Change**. The `Coordinator` dynamically moves the entity's memory from Archetype A to Archetype B.

## Why Archetypes?
In traditional ECS implementations (like sparse sets), iterating through components requires checking if an entity has the component, and potentially jumping around in memory.

With Archetypes, systems iterate over perfectly packed, contiguous arrays of data. This means the CPU cache is fully utilized, resulting in massive performance gains for large-scale games.

## System Dependency Graph
EC++ includes a built-in topological sorter for systems.

```cpp
gCoordinator.AddSystemDependency<PhysicsSystem, RenderSystem>();
```
This guarantees `PhysicsSystem` will always finish executing before `RenderSystem` starts, preventing race conditions and order-of-operation bugs without manual orchestration.
