#include <iostream>
#include <iomanip>
#include "../include/ecpp/Coordinator.hpp"
#include "../include/ecpp/Reflection.hpp"

// Components
struct Transform {
    float x, y, z;
};

struct Physics {
    float mass;
    float friction;
    bool isStatic;
};

// Reflect the components
ECPP_REFLECT_BEGIN(Transform)
    ECPP_REFLECT_FIELD(Transform, x),
    ECPP_REFLECT_FIELD(Transform, y),
    ECPP_REFLECT_FIELD(Transform, z)
ECPP_REFLECT_END()

ECPP_REFLECT_BEGIN(Physics)
    ECPP_REFLECT_FIELD(Physics, mass),
    ECPP_REFLECT_FIELD(Physics, friction),
    ECPP_REFLECT_FIELD(Physics, isStatic)
ECPP_REFLECT_END()

// A generic function that acts like a Game Engine UI Editor (e.g., Unity Inspector)
template <typename T>
void DrawInspector(const std::string& componentName, T* componentData) {
    std::cout << "--- " << componentName << " ---\n";
    
    // Iterate over the reflected fields dynamically
    for (const auto& field : ecpp::ReflectionInfo<T>::GetFields()) {
        std::cout << "  [" << field.typeName << "] " << std::left << std::setw(10) << field.name << " = ";
        
        // In a real editor, you'd use ImGui::DragFloat, ImGui::Checkbox, etc. based on field.typeName
        // Here we just use the raw memory offset to read the value
        void* fieldPtr = (uint8_t*)componentData + field.offset;

        if (field.typeName == "f") { // 'f' is mangled name for float on GCC/Clang
            std::cout << *(float*)fieldPtr;
        } else if (field.typeName == "b") { // 'b' is mangled name for bool
            std::cout << (*(bool*)fieldPtr ? "true" : "false");
        } else {
            std::cout << "<unknown type>";
        }
        std::cout << "\n";
    }
}

int main() {
    ecpp::Coordinator coord;
    coord.Init();

    coord.RegisterComponent<Transform>();
    coord.RegisterComponent<Physics>();

    ecpp::Entity player = coord.CreateEntity();
    coord.AddComponent(player, Transform{10.5f, 20.0f, -5.2f});
    coord.AddComponent(player, Physics{75.0f, 0.8f, false});

    std::cout << "=== Mock UI Editor (ImGui) ===\n";
    std::cout << "Selected Entity: " << player << "\n\n";

    // Imagine the UI looping over all components attached to the selected entity
    if (coord.HasComponent<Transform>(player)) {
        Transform& t = coord.GetComponent<Transform>(player);
        DrawInspector(ecpp::ReflectionInfo<Transform>::GetName(), &t);
    }

    if (coord.HasComponent<Physics>(player)) {
        Physics& p = coord.GetComponent<Physics>(player);
        DrawInspector(ecpp::ReflectionInfo<Physics>::GetName(), &p);
    }

    return 0;
}
