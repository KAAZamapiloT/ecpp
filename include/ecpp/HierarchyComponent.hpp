#pragma once

#include "Types.hpp"

namespace ecpp {

// A built-in component used to define entity relationships.
// It allows treating entities as a tree structure.
struct HierarchyComponent {
    Entity parent = INVALID_ENTITY;
    Entity firstChild = INVALID_ENTITY;
    Entity nextSibling = INVALID_ENTITY;
    Entity prevSibling = INVALID_ENTITY;

    // If true, this entity will not be destroyed when its parent is destroyed.
    // Instead, it will be orphaned (parent set to INVALID_ENTITY) or potentially
    // you can handle it manually.
    bool skipOnParentDestroy = false;
};

} // namespace ecpp
