#include <iostream>
#include <fstream>
#include "../include/ecpp/Coordinator.hpp"
#include "../include/ecpp/Serializer.hpp"

// Define some standard components
struct Transform {
    float x, y;
};

struct PlayerInfo {
    int health;
    int ammo;
};

struct WeaponInfo {
    int damage;
    bool isMelee;
};

// Reflect them for serialization
ECPP_REFLECT_BEGIN(Transform)
    ECPP_REFLECT_FIELD(Transform, x),
    ECPP_REFLECT_FIELD(Transform, y)
ECPP_REFLECT_END()

ECPP_REFLECT_BEGIN(PlayerInfo)
    ECPP_REFLECT_FIELD(PlayerInfo, health),
    ECPP_REFLECT_FIELD(PlayerInfo, ammo)
ECPP_REFLECT_END()

ECPP_REFLECT_BEGIN(WeaponInfo)
    ECPP_REFLECT_FIELD(WeaponInfo, damage),
    ECPP_REFLECT_FIELD(WeaponInfo, isMelee)
ECPP_REFLECT_END()

int main() {
    ecpp::Coordinator coord;
    coord.Init();

    // 1. Register Components
    coord.RegisterComponent<Transform>();
    coord.RegisterComponent<PlayerInfo>();
    coord.RegisterComponent<WeaponInfo>();

    // 2. Initialize Serializer
    ecpp::Serializer serializer;
    serializer.RegisterComponentSerializer<Transform>(coord);
    serializer.RegisterComponentSerializer<PlayerInfo>(coord);
    serializer.RegisterComponentSerializer<WeaponInfo>(coord);

    // 3. Create Hierarchical Entities
    std::cout << "Creating Player (Entity 0)...\n";
    ecpp::Entity player = coord.CreateEntity();
    coord.AddComponent(player, Transform{100.0f, 200.0f});
    coord.AddComponent(player, PlayerInfo{100, 50});

    std::cout << "Creating Sword (Entity 1) and attaching to Player...\n";
    ecpp::Entity sword = coord.CreateEntity();
    coord.AddComponent(sword, Transform{5.0f, 0.0f}); // Local offset from player
    coord.AddComponent(sword, WeaponInfo{25, true});
    
    coord.AddChild(player, sword);

    std::cout << "Creating Gun (Entity 2) and attaching to Player...\n";
    ecpp::Entity gun = coord.CreateEntity();
    coord.AddComponent(gun, Transform{-5.0f, 0.0f});
    coord.AddComponent(gun, WeaponInfo{15, false});
    
    coord.AddChild(player, gun);

    // 4. Serialize the entire game state to JSON!
    std::string saveFile = "savegame.json";
    std::cout << "\nSaving Game State to " << saveFile << "...\n";
    serializer.SaveToJSON(coord, saveFile);

    // 5. Read back the file to show it worked
    std::cout << "\n=== " << saveFile << " Contents ===\n";
    std::ifstream file(saveFile);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            std::cout << line << "\n";
        }
        file.close();
    }

    return 0;
}
