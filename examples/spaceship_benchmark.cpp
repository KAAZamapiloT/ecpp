#include <iostream>
#include <chrono>
#include <vector>
#include "../include/ecpp/Coordinator.hpp"

// Components
struct Position { float x, y; };
struct Velocity { float dx, dy; };
struct Sprite { uint32_t color; };
struct Health { int current; };

// Systems
class MovementSystem : public ecpp::System {
public:
    void Update(ecpp::Coordinator& coord, float dt) override {
        auto typePos = coord.GetComponentType<Position>();
        auto typeVel = coord.GetComponentType<Velocity>();

        for (auto* arch : mArchetypes) {
            for (size_t row = 0; row < arch->GetEntityCount(); ++row) {
                Position& pos = *(Position*)arch->GetComponentPtr(typePos, row);
                Velocity& vel = *(Velocity*)arch->GetComponentPtr(typeVel, row);

                pos.x += vel.dx * dt;
                pos.y += vel.dy * dt;
            }
        }
    }
};

class BoundarySystem : public ecpp::System {
public:
    void Update(ecpp::Coordinator& coord, float dt) override {
        auto typePos = coord.GetComponentType<Position>();
        auto typeVel = coord.GetComponentType<Velocity>();

        for (auto* arch : mArchetypes) {
            for (size_t row = 0; row < arch->GetEntityCount(); ++row) {
                Position& pos = *(Position*)arch->GetComponentPtr(typePos, row);
                Velocity& vel = *(Velocity*)arch->GetComponentPtr(typeVel, row);

                // Bounce off walls
                if (pos.x < 0.0f || pos.x > 1000.0f) vel.dx = -vel.dx;
                if (pos.y < 0.0f || pos.y > 1000.0f) vel.dy = -vel.dy;
            }
        }
    }
};

int main() {
    ecpp::Coordinator coord;
    coord.Init();

    coord.RegisterComponent<Position>();
    coord.RegisterComponent<Velocity>();
    coord.RegisterComponent<Sprite>();
    coord.RegisterComponent<Health>();

    auto moveSys = coord.RegisterSystem<MovementSystem>();
    ecpp::Signature moveSig;
    moveSig.set(coord.GetComponentType<Position>());
    moveSig.set(coord.GetComponentType<Velocity>());
    coord.SetSystemSignature<MovementSystem>(moveSig);

    auto boundSys = coord.RegisterSystem<BoundarySystem>();
    coord.SetSystemSignature<BoundarySystem>(moveSig); // Same signature

    std::cout << "Spawning 4,000 spaceships...\n";
    for (int i = 0; i < 4000; ++i) {
        ecpp::Entity e = coord.CreateEntity();
        coord.AddComponent(e, Position{ (float)(i % 1000), (float)(i % 1000) });
        coord.AddComponent(e, Velocity{ 10.0f, 15.0f });
        coord.AddComponent(e, Sprite{ 0xFF0000 });
        
        // Only half of them get health
        if (i % 2 == 0) {
            coord.AddComponent(e, Health{ 100 });
        }
    }

    std::cout << "Simulating 100 frames of movement for 4,000 entities...\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int frame = 0; frame < 100; ++frame) {
        moveSys->Update(coord, 0.016f);
        boundSys->Update(coord, 0.016f);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms = end - start;
    
    std::cout << "Done! Total time: " << ms.count() << " ms\n";
    std::cout << "Average time per frame: " << ms.count() / 100.0 << " ms\n";

    return 0;
}
