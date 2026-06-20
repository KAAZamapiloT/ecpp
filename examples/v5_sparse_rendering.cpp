#include <iostream>
#include <vector>
#include "../include/ecpp/Coordinator.hpp"
#include "../include/ecpp/SpatialHash.hpp"

struct Position { float x, y; };

// Zero-Byte Tag Component! Used to flag entities as 'currently visible'.
struct IsVisible {}; 

// A specialized render system that ONLY iterates over visible entities.
// Thanks to Archetype storage, zero-byte tags force the ECS to physically group
// all visible entities tightly together in memory, making rendering blindingly fast.
class RenderSystem : public ecpp::System {
public:
    void Update(ecpp::Coordinator& coord) {
        auto typePos = coord.GetComponentType<Position>();
        int renderCount = 0;

        for (auto* arch : mArchetypes) {
            // Because the system signature strictly requires IsVisible, we know 100% of these entities are visible.
            // There are NO branch prediction failures here.
            for (size_t row = 0; row < arch->GetEntityCount(); ++row) {
                Position& pos = *(Position*)arch->GetComponentPtr(typePos, row);
                // e.g., DrawSprite(pos.x, pos.y);
                renderCount++;
            }
        }
        std::cout << "[RenderSystem] Rendered " << renderCount << " entities directly from cache.\n";
    }
};

int main() {
    ecpp::Coordinator coord;
    coord.Init();

    coord.RegisterComponent<Position>();
    coord.RegisterComponent<IsVisible>();

    auto renderSys = coord.RegisterSystem<RenderSystem>();
    ecpp::Signature renderSig;
    renderSig.set(coord.GetComponentType<Position>());
    renderSig.set(coord.GetComponentType<IsVisible>()); // Key feature: Filtering by empty Tag
    coord.SetSystemSignature<RenderSystem>(renderSig);

    // Initialize standalone Spatial Hash with cell size of 100 world units
    ecpp::SpatialHash grid(100.0f);

    // Simulate 4,000 objects spread across a massive map
    std::cout << "Generating 4,000 objects across massive map...\n";
    for (int i = 0; i < 4000; ++i) {
        ecpp::Entity e = coord.CreateEntity();
        float x = (i * 15.5f);
        float y = (i * 20.1f);
        
        coord.AddComponent(e, Position{x, y});
        
        // Feed position into our generic standalone spatial grid
        grid.UpdateEntity(e, x, y);
    }

    // Imagine the Camera is looking at a small chunk of the map (X: 1000->1500, Y: 1000->1500)
    std::cout << "Camera moved to bounds (1000, 1000) -> (1500, 1500)\n";
    
    // 1. Instantly query the grid (culling millions of entities instantly)
    std::vector<ecpp::Entity> visibleEntities = grid.QueryAABB(1000.0f, 1000.0f, 1500.0f, 1500.0f);
    std::cout << "Grid returned " << visibleEntities.size() << " entities within camera bounds.\n";

    // 2. Tag them in the ECS
    for (ecpp::Entity e : visibleEntities) {
        coord.AddComponent(e, IsVisible{});
    }

    // 3. Render only the visible entities. The ECS has physically grouped them in cache!
    renderSys->Update(coord);

    // 4. Untag them when the camera moves away
    for (ecpp::Entity e : visibleEntities) {
        coord.RemoveComponent<IsVisible>(e);
    }

    std::cout << "Execution finished successfully.\n";
    return 0;
}
