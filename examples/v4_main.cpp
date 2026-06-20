#include <iostream>
#include <thread>
#include "../include/ecpp/Coordinator.hpp"
#include "../include/ecpp/CommandBuffer.hpp"
#include "../include/ecpp/Memory.hpp"
#include "../include/ecpp/Reflection.hpp"

// Components
struct Health {
    int max;
    int current;
};

struct Damage {
    int amount;
};

// Reflect Health for our imaginary Editor UI
ECPP_REFLECT_BEGIN(Health)
    ECPP_REFLECT_FIELD(Health, max),
    ECPP_REFLECT_FIELD(Health, current)
ECPP_REFLECT_END()

// Example System that safely queues ECS mutations from multiple threads
class DamageSystem : public ecpp::System {
public:
    void Update(ecpp::Coordinator& coord, ecpp::CommandBuffer& cmdBuf) {
        auto typeHealth = coord.GetComponentType<Health>();
        auto typeDamage = coord.GetComponentType<Damage>();

        // We pretend this is happening inside a JobSystem across multiple threads.
        // It's unsafe to call coord.DestroyEntity() while iterating archetypes.
        // So we use the CommandBuffer!
        for (auto* arch : mArchetypes) {
            for (size_t row = 0; row < arch->GetEntityCount(); ++row) {
                ecpp::Entity entity = arch->entities[row];
                Health& h = *(Health*)arch->GetComponentPtr(typeHealth, row);
                Damage& d = *(Damage*)arch->GetComponentPtr(typeDamage, row);

                h.current -= d.amount;
                std::cout << "Entity " << entity << " took " << d.amount << " damage! Health: " << h.current << "/" << h.max << "\n";

                if (h.current <= 0) {
                    std::cout << "Entity " << entity << " queued for destruction.\n";
                    cmdBuf.QueueDestroyEntity(entity);
                } else {
                    // Remove damage component so it only applies once
                    cmdBuf.QueueRemoveComponent<Damage>(entity);
                }
            }
        }
    }
};

int main() {
    ecpp::Coordinator coord;
    coord.Init();

    coord.RegisterComponent<Health>();
    coord.RegisterComponent<Damage>();

    auto damageSys = coord.RegisterSystem<DamageSystem>();
    ecpp::Signature sig;
    sig.set(coord.GetComponentType<Health>());
    sig.set(coord.GetComponentType<Damage>());
    coord.SetSystemSignature<DamageSystem>(sig);

    ecpp::CommandBuffer cmdBuf;

    // 1. Showcase Reflection (Printing out Editor UI Data)
    std::cout << "=== V4 Reflection System ===\n";
    std::cout << "Inspecting Struct: " << ecpp::ReflectionInfo<Health>::GetName() << "\n";
    for (const auto& field : ecpp::ReflectionInfo<Health>::GetFields()) {
        std::cout << "  - Field Name: " << field.name 
                  << " | Type: " << field.typeName 
                  << " | Offset: " << field.offset 
                  << " | Size: " << field.size << " bytes\n";
    }
    std::cout << "\n";

    // 2. Showcase Allocators
    std::cout << "=== V4 Memory Allocators ===\n";
    ecpp::LinearAllocator frameAlloc(1024 * 1024); // 1MB frame buffer
    void* tempPtr = frameAlloc.Allocate(256);
    std::cout << "Allocated 256 bytes from LinearAllocator at: " << tempPtr << "\n";
    frameAlloc.Reset(); // Instantly frees everything

    ecpp::PoolAllocator bulletPool(sizeof(void*), 100);
    void* b1 = bulletPool.Allocate();
    void* b2 = bulletPool.Allocate();
    std::cout << "Allocated from PoolAllocator: " << b1 << ", " << b2 << "\n";
    bulletPool.Free(b1);
    bulletPool.Free(b2);
    std::cout << "\n";

    // 3. Showcase Command Buffers
    std::cout << "=== V4 Command Buffers ===\n";
    ecpp::Entity e1 = coord.CreateEntity();
    coord.AddComponent(e1, Health{100, 100});
    coord.AddComponent(e1, Damage{50});

    ecpp::Entity e2 = coord.CreateEntity();
    coord.AddComponent(e2, Health{30, 30});
    coord.AddComponent(e2, Damage{40}); // This will kill e2

    std::cout << "Running multithread-safe DamageSystem...\n";
    damageSys->Update(coord, cmdBuf);

    std::cout << "Executing queued commands...\n";
    cmdBuf.Execute(coord); // Applies the damage removals and destroys e2 safely

    std::cout << "Entities remaining:\n";
    std::cout << "e1 exists: " << (coord.HasComponent<Health>(e1) ? "Yes" : "No") << "\n";
    
    // Check if e2 exists. Since it was destroyed, querying its components should either throw or we just trust the system.
    // In our ECS, destroyed entities' components are inaccessible.
    std::cout << "Execution finished successfully.\n";

    return 0;
}
