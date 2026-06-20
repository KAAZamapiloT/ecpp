#include "../include/ecpp/Coordinator.hpp"
#include "../include/ecpp/JobSystem.hpp"
#include "../include/ecpp/ESync.hpp"
#include "../include/ecpp/Serializer.hpp"
#include <iostream>
#include <string>

using namespace ecpp;

// Custom Components
struct Health {
    int hp;
    EMutex mutex; // For thread-safe modifications
};

struct DamageEvent {
    Entity target;
    int damageAmount;
};

// Singleton
struct GameState {
    bool isRunning = true;
    int score = 0;
};

int main() {
    std::cout << "--- EC++ V2 Advanced Example ---\n\n";

    Coordinator coordinator;
    coordinator.Init();

    // 1. Singletons
    coordinator.SetSingleton<GameState>(GameState{true, 100});
    std::cout << "Singleton GameState score: " << coordinator.GetSingleton<GameState>().score << "\n\n";

    // 2. Events
    coordinator.RegisterComponent<Health>();
    
    // Listen to DamageEvent
    coordinator.AddEventListener<DamageEvent>([&coordinator](const DamageEvent& event) {
        if (coordinator.HasComponent<Health>(event.target)) {
            auto& health = coordinator.GetComponent<Health>(event.target);
            ELock lock(health.mutex); // Thread-safe lock
            health.hp -= event.damageAmount;
            std::cout << "Event Received: Entity " << event.target << " took " << event.damageAmount 
                      << " damage. HP left: " << health.hp << "\n";
        }
    });

    Entity player = coordinator.CreateEntity();
    coordinator.AddComponent(player, Health{100});

    std::cout << "Emitting Damage Event...\n";
    coordinator.EmitEvent(DamageEvent{player, 25});

    // 3. Views
    Entity enemy1 = coordinator.CreateEntity();
    coordinator.AddComponent(enemy1, Health{50});
    
    std::cout << "\nIterating entities with Health using Views...\n";
    auto entitiesWithHealth = coordinator.GetEntitiesWith<Health>();
    for (Entity e : entitiesWithHealth) {
        auto& h = coordinator.GetComponent<Health>(e);
        std::cout << "Entity " << e << " has " << h.hp << " HP.\n";
    }

    // 4. Multithreading / JobSystem
    std::cout << "\nDispatching Parallel Jobs...\n";
    JobSystem jobSystem;
    jobSystem.Init(); // Initializes thread pool
    
    // We can process the entitiesWithHealth array in parallel
    jobSystem.Dispatch(entitiesWithHealth.size(), [&](uint32_t start, uint32_t end) {
        for (uint32_t i = start; i < end; ++i) {
            Entity e = entitiesWithHealth[i];
            auto& h = coordinator.GetComponent<Health>(e);
            
            ELock lock(h.mutex);
            h.hp += 1; // Everyone heals by 1
            // Note: std::cout is not thread-safe, so output might interleave, but logic is safe.
        }
    });

    std::cout << "Parallel healing completed.\n";
    for (Entity e : entitiesWithHealth) {
        std::cout << "Entity " << e << " has " << coordinator.GetComponent<Health>(e).hp << " HP.\n";
    }

    // 5. Serialization
    std::cout << "\nTesting Serialization...\n";
    Serializer serializer(coordinator);
    
    // Register how to save and load Health component
    serializer.RegisterComponent<Health>(
        [](const Health& h, std::ostream& os) {
            os.write(reinterpret_cast<const char*>(&h.hp), sizeof(int));
        },
        [](Health& h, std::istream& is) {
            is.read(reinterpret_cast<char*>(&h.hp), sizeof(int));
        }
    );

    std::string filepath = "savegame.bin";
    serializer.SaveToFile(filepath);
    std::cout << "Saved state to " << filepath << ".\n";

    // Destroy all entities to simulate loading a fresh game
    coordinator.DestroyEntity(player);
    coordinator.DestroyEntity(enemy1);
    std::cout << "Destroyed all entities. Living entities: " << coordinator.GetActiveEntities().size() << "\n";

    // Load from file
    auto idMap = serializer.LoadFromFile(filepath);
    std::cout << "Loaded state from " << filepath << ".\n";
    
    for (const auto& pair : idMap) {
        std::cout << "Old Entity " << pair.first << " is now New Entity " << pair.second << "\n";
        auto& h = coordinator.GetComponent<Health>(pair.second);
        std::cout << "  -> Loaded HP: " << h.hp << "\n";
    }

    std::cout << "\nEC++ V2 advanced demo finished successfully.\n";
    return 0;
}
