# Advanced Engine Features (V4)

EC++ V4 introduced powerful tools specifically designed for Game Engine developers building complex systems on top of the ECS.

## 1. Command Buffers (Thread Safety)

When you parallelize your Systems using a Job System or Thread Pool, you cannot safely destroy entities or add/remove components while iterating over Archetypes. Doing so will cause memory corruption.

The `CommandBuffer` solves this by recording mutations and safely playing them back later.

```cpp
// 1. Pass a CommandBuffer to your worker threads
void WorkerThread(ecpp::Coordinator& coord, ecpp::CommandBuffer& cmdBuf) {
    // 2. Queue mutations safely
    cmdBuf.QueueDestroyEntity(someEntity);
    cmdBuf.QueueAddComponent<Damage>(someEntity, Damage{50});
}

// 3. Back on the main thread, execute all queued mutations
cmdBuf.Execute(coord);
```

## 2. Memory Allocators

Game engines must tightly control memory allocations to prevent heap fragmentation and `malloc` overhead.

### Linear Allocator
The `LinearAllocator` is an extremely fast bump-pointer allocator. Use it for temporary data generated during a single frame. It clears instantly.

```cpp
ecpp::LinearAllocator frameAlloc(1024 * 1024); // 1MB buffer
void* tempMemory = frameAlloc.Allocate(256);
// ... end of frame ...
frameAlloc.Reset(); // O(1) clear
```

### Pool Allocator
The `PoolAllocator` is perfect for spawning and destroying millions of identical objects (like bullets) without fragmenting the heap.

```cpp
// Allocate a pool for 10,000 blocks the size of a generic pointer
ecpp::PoolAllocator bulletPool(sizeof(void*), 10000);

void* bulletMemory = bulletPool.Allocate();
bulletPool.Free(bulletMemory);
```

## 3. Reflection & Meta-Programming

Building an Editor UI (like Unity's Inspector) requires reading component fields at runtime. C++ does not support this natively. EC++ provides a zero-dependency macro solution.

### Registering a Struct
```cpp
struct Transform {
    float x, y, z;
};

// Use the macros to reflect the struct
ECPP_REFLECT_BEGIN(Transform)
    ECPP_REFLECT_FIELD(Transform, x),
    ECPP_REFLECT_FIELD(Transform, y),
    ECPP_REFLECT_FIELD(Transform, z)
ECPP_REFLECT_END()
```

### Using Reflection Data
You can now iterate over the struct's fields dynamically at runtime. This allows you to draw UI sliders for *any* component without writing custom UI code!

```cpp
for (const auto& field : ecpp::ReflectionInfo<Transform>::GetFields()) {
    std::cout << "Field Name: " << field.name << "\n";
    std::cout << "Type Name: " << field.typeName << "\n";
    std::cout << "Memory Offset: " << field.offset << "\n";
}
```
