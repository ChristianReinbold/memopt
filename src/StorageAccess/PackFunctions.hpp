#pragma once

#include "StorageAccess.hpp"

struct PackFunctions {
  // Assuming that the pack has not been loaded yet, it is filtered out in case
  // storing it now would result in more register pressure than storing it the
  // next time a variable of the pack is loaded from memory.
  //
  // Required preconditions for values passed in: pack(v) will not be modified
  // anymore. This implies: If A2::isUsed changes in the future, then it is
  // because of a read operation that may load the pack.
  template <typename Storage, typename A1, typename A2>
  static constexpr uint filterOptimalStore(uint pack, uint v) {
    // Storing now means that all the values that are loaded to obtain a
    // complete pack for storing will be kept alive if they are used later.
    // Hence, we need registers for those.
    uint nRegistersStoreNow = Storage::sizeOf(A2::isUsed & pack);
    // Storing later means that we have to keep the values we intend to store
    // alive until then.
    uint nRegistersStoreLater = Storage::sizeOf(A1::wasModified & pack);
    // Delay the store operation if storing now will result in more register
    // pressure
    return (nRegistersStoreNow > nRegistersStoreLater) ? (v & ~pack) : v;
  }

  static constexpr uint complete(uint pack, uint v) {
    return (pack & v) ? v | pack : v;
  }
};