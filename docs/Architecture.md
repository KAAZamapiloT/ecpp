[Home](../README.md) | [Getting Started](GettingStarted.md) | **Architecture** | [Advanced Features](AdvancedFeatures.md) | [API Reference](API_Reference.md)

---

# The EC++ Architecture (Under the Hood)

Understanding how EC++ manages memory will help you write games that can process millions of entities at 60 Frames Per Second.

## The CPU Cache Problem

In traditional engines (like standard Unity `MonoBehaviour` scripts), when the CPU wants to process a `Player` object, it asks RAM for the data. RAM is extremely slow. 

To hide this, CPUs load data into a tiny, ultra-fast memory pool called the **L1 Cache**. The catch? It loads data in continuous "lines". If your `Player` object is scattered randomly across RAM, the CPU has to keep asking RAM for new data, causing a **Cache Miss**. This is why OOP engines struggle to have 10,000 enemies on screen.

## The EC++ Solution: Archetype Storage

EC++ doesn't store "Objects". It uses **Archetypes**.
An Archetype is a giant, contiguous array in memory that strictly holds entities with the *exact same combination of components*.

### Use Case Example: 
Imagine you spawn 1,000 `Zombie` entities (Position + Health) and 5,000 `Bullet` entities (Position + Velocity).

*   **Archetype A (Position, Health)**: Holds all 1,000 Zombies tightly packed side-by-side.
*   **Archetype B (Position, Velocity)**: Holds all 5,000 Bullets tightly packed side-by-side.

When your `PhysicsSystem` asks for entities with `Position` and `Velocity`, EC++ doesn't search through 6,000 random entities. It completely ignores Archetype A, goes straight to Archetype B, and hands the CPU a perfectly dense array of memory. The CPU iterates through all 5,000 bullets without a single cache miss. 

**This allows you to simulate millions of entities in milliseconds.**

## Zero-Byte Tagging (Sparse Rendering)

Because Archetypes physically group identical entities together in memory, you can abuse this feature for massive performance gains using **Tag Components**.

A Tag Component is an empty struct: `struct IsVisible {};`

### Use Case Example: Frustum Culling
You have an open-world game with 100,000 trees. Only 50 trees are currently visible on the player's screen.
If your Render System iterates through 100,000 trees just to check `if(tree.isOnScreen)`, your game will lag.

**The ECS Way:**
1. You have a lightweight system that checks bounds. When a tree enters the screen, it does: `coord.AddComponent(tree, IsVisible{});`
2. Under the hood, EC++ instantly pulls that tree out of the "Invisible" Archetype memory block, and physically copies it into the "Visible" Archetype memory block.
3. Your heavy `RenderSystem` requires `IsVisible` in its Signature. 
4. The CPU directly iterates the "Visible" Archetype. It processes exactly 50 trees. No `if` statements, no branch prediction failures, zero cache misses.

## System Dependency Graphs (Topological Sorting)

In a complex engine, the order in which Systems execute matters immensely.
If your `RenderSystem` runs *before* your `PhysicsSystem`, the player will experience a 1-frame visual delay because the screen drew the objects *before* they moved!

EC++ solves this with a built-in Dependency Graph.

```cpp
// Tell the engine that Rendering MUST happen after Physics
coord.AddSystemDependency<RenderSystem, PhysicsSystem>();
```

When you call `coord.UpdateSystems()`, the engine automatically builds a Topological Graph and guarantees that Physics will completely finish executing before Rendering begins.
