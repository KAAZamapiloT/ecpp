#pragma once

#include <cstdint>

namespace ecpp {

// A built-in component for multiplayer networking.
// Game engines can use this to sync entities across the network.
struct NetworkComponent {
    // Unique identifier for this entity across the network.
    std::uint32_t networkId = 0;

    // The client ID of the player who "owns" or controls this entity.
    // E.g., 0 for server, 1-N for connected clients.
    std::uint32_t ownerClientId = 0;

    // True if this instance of the game is the authority or owner of this entity.
    // Useful for client-side prediction or determining who sends updates.
    bool isLocalPlayer = false;
};

} // namespace ecpp
