#pragma once

#include <Kernel.hpp>
#include <StorageAccess/PackFunctions.hpp>

struct Vars
{
    uint a1, a2, b, c1, c2, d, e, f, g;
};

struct Var
{
    enum Enum : uint {
        A1 = 1u << 0,
        A2 = 1u << 1,
        B  = 1u << 2,
        C1 = 1u << 3,
        C2 = 1u << 4,
        D  = 1u << 5,
        E  = 1u << 6,
        F  = 1u << 7,
        G  = 1u << 8
    };
};

struct Pack
{
    enum Enum : uint {
        A = Var::A1 | Var::A2,
        B = Var::B,
        C = Var::C1 | Var::C2,
        D = Var::D,
        E = Var::E,
        F = Var::F,
        G = Var::G
    };
};

struct Storage {
    using Var = ::Var::Enum;
    using Pack = ::Pack::Enum;

    static constexpr uint sizeOf(uint vars) {
        return 
            1 * (bool)(vars & Var::A1) + 
            1 * (bool)(vars & Var::A2) + 
            1 * (bool)(vars & Var::B) + 
            1 * (bool)(vars & Var::C1) + 
            1 * (bool)(vars & Var::C2) + 
            1 * (bool)(vars & Var::D) + 
            1 * (bool)(vars & Var::E) + 
            1 * (bool)(vars & Var::F) + 
            1 * (bool)(vars & Var::G);
    }

    template <typename A1, typename A2>
    static constexpr uint filterStoredPacks(uint v) {
        uint v1 = PackFunctions::filterOptimalStore<Storage, A1, A2>(Pack::A, v);
        uint v2 = PackFunctions::filterOptimalStore<Storage, A1, A2>(Pack::C, v1);
        return v2;
    }

    static constexpr uint pack(uint v) {
        uint v1 = PackFunctions::complete(Pack::A, v);
        uint v2 = PackFunctions::complete(Pack::C, v1);
        return v2;
    }

    void load(inout(Vars) dst, uint loadedPacks = 0, uint loadedVars = 0);
    void store(inout(Vars) src, uint storedPacks = 0);
};