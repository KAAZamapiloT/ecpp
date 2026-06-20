#include "../include/ecpp/Coordinator.hpp"
#include "../include/ecpp/DynamicComponentManager.hpp"
#include "../include/ecpp/NetworkState.hpp"
#include <iostream>
#include <string>

using namespace ecpp;

struct Position { float x, y; };
struct Velocity { float dx, dy; };
struct Name { std::string name; }; // Tests deep string copying/moving inside Archetypes

class PhysicsSystem : public System {
public:
    void Update(Coordinator& coordinator, float dt) override {
        auto typePos = coordinator.GetComponentType<Position>();
        auto typeVel = coordinator.GetComponentType<Velocity>();

        // V3 Archetype iteration loop (Maximum Cache Performance)
        for (Archetype* arch : mArchetypes) {
            for (size_t row = 0; row < arch->GetEntityCount(); ++row) {
                Position& pos = *(Position*)arch->GetComponentPtr(typePos, row);
                Velocity& vel = *(Velocity*)arch->GetComponentPtr(typeVel, row);
                pos.x += vel.dx * dt;
                pos.y += vel.dy * dt;
            }
        }
    }
};

class PrintSystem : public System {
public:
    void Update(Coordinator& coordinator, float dt) override {
        auto typePos = coordinator.GetComponentType<Position>();
        auto typeName = coordinator.GetComponentType<Name>();

        for (Archetype* arch : mArchetypes) {
            for (size_t row = 0; row < arch->GetEntityCount(); ++row) {
                Position& pos = *(Position*)arch->GetComponentPtr(typePos, row);
                Name& name = *(Name*)arch->GetComponentPtr(typeName, row);
                std::cout << name.name << " is at (" << pos.x << ", " << pos.y << ")\n";
            }
        }
    }
};

int main() {
    std::cout << "--- EC++ V3 AAA Example ---\n\n";

    Coordinator coordinator;
    coordinator.Init();

    coordinator.RegisterComponent<Position>();
    coordinator.RegisterComponent<Velocity>();
    coordinator.RegisterComponent<Name>();

    // 1. Reactive Hooks & Network State Tracker
    NetworkState networkState(coordinator);
    networkState.TrackComponentChanges<Position>();

    coordinator.OnComponentAdded<Name>([](Entity e, Name& name) {
        std::cout << "HOOK FIRED: Entity " << e << " was given the name '" << name.name << "'\n";
    });

    // 2. Setup Entities
    Entity player = coordinator.CreateEntity();
    coordinator.AddComponent(player, Position{0.0f, 0.0f});
    coordinator.AddComponent(player, Velocity{10.0f, 5.0f});
    
    // Triggers Structural Change: Memory moves from {Pos, Vel} Archetype to {Pos, Vel, Name} Archetype.
    coordinator.AddComponent(player, Name{"Hero"});

    // 3. System Graph
    auto physicsSys = coordinator.RegisterSystem<PhysicsSystem>();
    Signature physSig;
    physSig.set(coordinator.GetComponentType<Position>());
    physSig.set(coordinator.GetComponentType<Velocity>());
    coordinator.SetSystemSignature<PhysicsSystem>(physSig);

    auto printSys = coordinator.RegisterSystem<PrintSystem>();
    Signature printSig;
    printSig.set(coordinator.GetComponentType<Position>());
    printSig.set(coordinator.GetComponentType<Name>());
    coordinator.SetSystemSignature<PrintSystem>(printSig);

    // Define Graph Dependency: PrintSystem ALWAYS runs AFTER PhysicsSystem
    coordinator.AddSystemDependency<PhysicsSystem, PrintSystem>();

    std::cout << "\n--- System Graph Execution ---\n";
    for (int i = 0; i < 3; ++i) {
        // Automatically resolves the dependency graph and calls Physics -> Print
        coordinator.UpdateSystems(1.0f);
    }

    // 4. Dynamic Component Manager (Scripting)
    std::cout << "\n--- Dynamic Components (Scripting Support) ---\n";
    DynamicComponentManager dynManager(coordinator);
    
    struct LuaData { int mana; };
    dynManager.RegisterDynamicComponent("LuaScriptData", sizeof(LuaData));
    
    LuaData myData{150};
    dynManager.AddDynamicComponent(player, "LuaScriptData", &myData);
    
    LuaData* retrievedData = (LuaData*)dynManager.GetDynamicComponent(player, "LuaScriptData");
    std::cout << "Dynamically retrieved mana: " << retrievedData->mana << "\n";

    std::cout << "\nEC++ V3 demo finished successfully.\n";
    return 0;
}
