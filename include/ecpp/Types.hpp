#pragma once

#include <bitset>
#include <cstdint>

namespace ecpp {

// ECS uses simple integers for Entities
using Entity = std::uint32_t;

// Max amount of entities we can have at any given time.
const Entity MAX_ENTITIES = 5000;

// Used to denote an invalid entity or no entity.
const Entity INVALID_ENTITY = MAX_ENTITIES;

// ComponentType is an ID assigned to each unique component structure at runtime.
using ComponentType = std::uint8_t;

// Maximum distinct component types that can be registered.
const ComponentType MAX_COMPONENTS = 64;

// Signature represents which components an entity has.
// A set bit at index i means the entity has the component with ComponentType i.
using Signature = std::bitset<MAX_COMPONENTS>;

} // namespace ecpp
