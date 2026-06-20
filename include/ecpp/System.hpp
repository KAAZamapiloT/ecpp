#pragma once

#include "Types.hpp"
#include "Archetype.hpp"
#include <vector>

namespace ecpp {

class Coordinator; // Forward declare

class System {
public:
    virtual ~System() = default;
    
    // The main update loop for this system
    virtual void Update(Coordinator& coordinator, float dt) {}
    
    // In V3, Systems iterate over Archetypes instead of individual entities for maximum speed.
    std::vector<Archetype*> mArchetypes;
};

} // namespace ecpp
