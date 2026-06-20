#include "../include/ecpp/Coordinator.hpp"
#include "../include/ecpp/EntitySpawner.hpp"
#include "../include/ecpp/NetworkComponent.hpp"
#include "../include/ecpp/HierarchyComponent.hpp"
#include <iostream>

using namespace ecpp;

// Custom Components
struct Transform {
    float x = 0.0f;
    float y = 0.0f;
};

struct RigidBody {
    float velocityX = 0.0f;
    float velocityY = 0.0f;
};

// Custom System
class PhysicsSystem : public System {
public:
    void Update(Coordinator& coordinator, float dt) {
        for (auto const& entity : mEntities) {
            auto& transform = coordinator.GetComponent<Transform>(entity);
            auto& rigidbody = coordinator.GetComponent<RigidBody>(entity);

            transform.x += rigidbody.velocityX * dt;
            transform.y += rigidbody.velocityY * dt;
        }
    }
};

int main() {
    std::cout << "--- EC++ Library Example ---\n\n";

    Coordinator coordinator;
    coordinator.Init();

    // Register our custom components
    coordinator.RegisterComponent<Transform>();
    coordinator.RegisterComponent<RigidBody>();

    // Register our NetworkComponent (built-in, but needs registration if used)
    coordinator.RegisterComponent<NetworkComponent>();

    // Register the PhysicsSystem
    auto physicsSystem = coordinator.RegisterSystem<PhysicsSystem>();

    // Set signature for PhysicsSystem (requires Transform and RigidBody)
    Signature physicsSignature;
    physicsSignature.set(coordinator.GetComponentType<Transform>());
    physicsSignature.set(coordinator.GetComponentType<RigidBody>());
    coordinator.SetSystemSignature<PhysicsSystem>(physicsSignature);


    // --- Entity Spawner Example ---
    EntitySpawner spawner(coordinator);
    
    // Register a "Player" prefab
    spawner.RegisterPrefab("Player", [](Entity entity, Coordinator& coord) {
        coord.AddComponent(entity, Transform{10.0f, 10.0f});
        coord.AddComponent(entity, RigidBody{5.0f, 0.0f});
        coord.AddComponent(entity, NetworkComponent{1001, 1, true});
    });

    std::cout << "Spawning Player entity...\n";
    Entity player1 = spawner.Spawn("Player");
    
    auto& p1Net = coordinator.GetComponent<NetworkComponent>(player1);
    std::cout << "Player 1 created with Network ID: " << p1Net.networkId 
              << ", Owner Client ID: " << p1Net.ownerClientId << "\n\n";


    // --- Hierarchy Example ---
    std::cout << "--- Testing Hierarchy ---\n";
    Entity parent = coordinator.CreateEntity();
    Entity child1 = coordinator.CreateEntity();
    Entity child2 = coordinator.CreateEntity();

    // Make child1 and child2 children of parent
    coordinator.AddChild(parent, child1);
    coordinator.AddChild(parent, child2);

    // Let's protect child2 from cascaded destruction
    auto& child2Hierarchy = coordinator.GetComponent<HierarchyComponent>(child2);
    child2Hierarchy.skipOnParentDestroy = true;

    std::cout << "Parent and 2 children created. Child 2 is protected (skipOnParentDestroy=true).\n";
    std::cout << "Destroying parent...\n";
    
    coordinator.DestroyEntity(parent);

    // parent and child1 should be destroyed, child2 should be alive but orphaned.
    // In this basic ECS, destroyed entities have their signature reset, 
    // so checking HasComponent is a good proxy for "is alive" if it had components, 
    // but a safer check here is just examining child2's new state.
    
    std::cout << "Child 2 parent after destruction: " << child2Hierarchy.parent << " (Expected: " << INVALID_ENTITY << ")\n";
    if (child2Hierarchy.parent == INVALID_ENTITY) {
        std::cout << "Child 2 successfully orphaned!\n";
    }

    // --- Simulating a game loop ---
    std::cout << "\n--- Physics Simulation ---\n";
    for (int i = 0; i < 3; ++i) {
        physicsSystem->Update(coordinator, 1.0f); // 1.0f dt
        auto& t = coordinator.GetComponent<Transform>(player1);
        std::cout << "Player 1 position: (" << t.x << ", " << t.y << ")\n";
    }

    std::cout << "\nEC++ demo finished successfully.\n";
    return 0;
}
