#pragma once

#include "StorageAccess.hpp"

#ifndef __HLSL__
struct PrintInfo;
#endif

struct DefaultA1
{
    // Variables for which previous blocks requested a store operation. This implies
    // that registers and global memory desynced at some point.
    static const uint requestedStore = 0u;

    // Variables that were modified by previous blocks. This attribute also
    // implies that the variables are available (see DefaultA3::isAvailable).
    static const uint wasModified = 0u;
};

struct DefaultA2 {
    // TODO: Rename, may conflict with "wasUsed" in interpretation...
    
    // Variables for which their current values will be read by future
    // blocks. If a variable is modified such that the current value goes out of
    // scope, this attribute is reset.
    static const uint isUsed = 0u;

    // Variables for which storing has to be delayed to a later block of the
    // control structure.
    static const uint blockStore = 0u;
};

struct DefaultA3 {
    // Variables for which their current values are held in registers.
    static const uint isAvailable = 0u;

    // Used to assert that each pack is loaded and stored at most once. These
    // attributes are not required to derive the correct storage access patterns
    // in the annotated control structure.
    static const uint loadedPacks = 0u;
    static const uint storedPacks = 0u;
};

template <typename Storage, typename A1, typename A2, typename A3>
struct Attributes
{
    /* TODO New names
        pRequireStore (requestedStore)
        vModified (wasModified)
        vRequired (isUsed) // Current value required later in control structure
        pCanStore (~blockStore)
        vAvailable (isAvailable)
        pLoaded (loadedPacks)
        pStored (storedPacks)

    */
    static const uint requestedStore = A1::requestedStore; // TODO: Convert to pack?
    static const uint wasModified = A1::wasModified;
    static const uint isUsed = A2::isUsed;
    static const uint blockStore = A2::blockStore; // TODO: Convert to pack?
    static const uint isAvailable = A3::isAvailable;
    static const uint loadedPacks = A3::loadedPacks;
    static const uint storedPacks = A3::storedPacks;

    #ifndef __HLSL__
    static bool isVisibleInStream(PrintInfo const&);
    template <typename Stream>
    static void stream(Stream& stream, PrintInfo const&);
    #endif
};