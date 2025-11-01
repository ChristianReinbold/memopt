#pragma once

#include "_Internal.hpp"
#include "StorageAccess.hpp"

struct EmptyBlock
{
    template <typename Storage, typename A1> struct Pass1;
    MAKE_ANNOTATABLE
};

template <typename Storage, typename A1>
struct EmptyBlock::Pass1
{
    template <typename A2> struct Pass2;
    using NextA1 = A1;
};

template <typename Storage, typename A1>
template <typename A2>
struct EmptyBlock::Pass1<Storage, A1>::Pass2
{
    template <typename A3> struct Pass3;
    static const uint requiredVars = 0u;
    using NextA2 = A2;
};

template <typename Storage, typename A1>
template <typename A2>
template <typename A3>
struct EmptyBlock::Pass1<Storage, A1>::Pass2<A2>::Pass3
{
    using NextA3 = A3;
    using InAttributes = Attributes<Storage, A1, A2, A3>;
    using OutAttributes = InAttributes;
    using ControlStructure = EmptyBlock;

    static Pass3 create(Storage storage) { return INIT(Pass3); }

#ifndef __HLSL__
    static bool isVisibleInStream(PrintInfo const&);
    template <typename Stream>
    static void stream(Stream&, PrintInfo const&);
#endif
};