#pragma once

#include "Coordinator.hpp"
#include "Reflection.hpp"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

namespace ecpp {

// A zero-dependency, lightweight JSON Serializer for EC++ Game States.
// Leverages the C++ Reflection macros to dynamically save ECS data.
class Serializer {
public:
    using SerializeFn = std::string(*)(void*);

    template<typename T>
    void RegisterComponentSerializer(Coordinator& coord) {
        ComponentType typeId = coord.GetComponentType<T>();
        
        mSerializers[typeId] = [](void* data) -> std::string {
            std::stringstream ss;
            ss << "{";
            
            auto fields = ReflectionInfo<T>::GetFields();
            for (size_t i = 0; i < fields.size(); ++i) {
                const auto& field = fields[i];
                void* fieldPtr = (uint8_t*)data + field.offset;
                
                ss << "\"" << field.name << "\":";

                if (field.typeName == "i") {
                    ss << *(int*)fieldPtr;
                } else if (field.typeName == "f") {
                    ss << *(float*)fieldPtr;
                } else if (field.typeName == "b") {
                    ss << (*(bool*)fieldPtr ? "true" : "false");
                } else {
                    ss << "0"; 
                }

                if (i < fields.size() - 1) ss << ",";
            }
            ss << "}";
            return ss.str();
        };

        mComponentNames[typeId] = ReflectionInfo<T>::GetName();
    }

    void SaveToJSON(Coordinator& coord, const std::string& filepath) {
        std::ofstream file(filepath);
        if (!file.is_open()) return;

        file << "{\n  \"entities\": [\n";

        bool firstEntity = true;
        for (Entity e = 0; e < MAX_ENTITIES; ++e) {
            if (!coord.IsAlive(e)) continue;

            if (!firstEntity) file << ",\n";
            file << "    {\n      \"id\": " << e << ",\n      \"components\": {\n";

            bool firstComponent = true;
            Signature sig = coord.GetEntitySignature(e);
            
            for (ComponentType typeId = 0; typeId < MAX_COMPONENTS; ++typeId) {
                if (sig.test(typeId) && mSerializers.find(typeId) != mSerializers.end()) {
                    if (!firstComponent) file << ",\n";
                    
                    void* rawPtr = coord.GetComponentRaw(e, typeId);
                    if (rawPtr) {
                        file << "        \"" << mComponentNames[typeId] << "\": " << mSerializers[typeId](rawPtr);
                        firstComponent = false;
                    }
                }
            }

            file << "\n      }\n    }";
            firstEntity = false;
        }

        file << "\n  ]\n}\n";
        file.close();
    }

private:
    std::unordered_map<ComponentType, SerializeFn> mSerializers;
    std::unordered_map<ComponentType, std::string> mComponentNames;
};

} // namespace ecpp
