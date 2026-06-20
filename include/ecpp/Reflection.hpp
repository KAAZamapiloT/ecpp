#pragma once

#include <string>
#include <vector>
#include <cstddef>
#include <typeinfo>

namespace ecpp {

struct FieldMetadata {
    std::string name;
    std::string typeName;
    size_t offset;
    size_t size;
};

template <typename T>
struct ReflectionInfo {
    static const char* GetName() { return "Unknown"; }
    static std::vector<FieldMetadata> GetFields() { return {}; }
};

} // namespace ecpp

// Core Reflection Macros for defining struct metadata without external code generation
#define ECPP_REFLECT_FIELD(StructType, FieldName) \
    ecpp::FieldMetadata{#FieldName, typeid(decltype(StructType::FieldName)).name(), offsetof(StructType, FieldName), sizeof(decltype(StructType::FieldName))}

#define ECPP_REFLECT_BEGIN(StructType) \
    namespace ecpp { \
    template <> struct ReflectionInfo<StructType> { \
        static const char* GetName() { return #StructType; } \
        static std::vector<FieldMetadata> GetFields() { \
            return {

#define ECPP_REFLECT_END() \
            }; \
        } \
    }; \
    }
