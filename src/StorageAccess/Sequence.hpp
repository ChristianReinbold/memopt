#pragma once

#include "_Internal.hpp"
#include "StorageAccess.hpp"

template <typename B1, typename B2, typename B3, typename B4, typename B5>
struct Sequence_
{
    template <typename Storage, typename A1> struct Pass1;
    MAKE_ANNOTATABLE
};

template <typename B1, typename B2, typename B3, typename B4, typename B5>
template <typename Storage, typename A1>
struct Sequence_<B1, B2, B3, B4, B5>::Pass1
{
    template <typename A2> struct Pass2;
    using B1A1  = A1;
    using B1Pass1  = typename B1::template Pass1<Storage, B1A1>;
    using B2A1 = typename B1Pass1::NextA1;
    using B2Pass1  = typename B2::template Pass1<Storage, B2A1>;
    using B3A1 = typename B2Pass1::NextA1;
    using B3Pass1  = typename B3::template Pass1<Storage, B3A1>;
    using B4A1 = typename B3Pass1::NextA1;
    using B4Pass1  = typename B4::template Pass1<Storage, B4A1>;
    using B5A1 = typename B4Pass1::NextA1;
    using B5Pass1  = typename B5::template Pass1<Storage, B5A1>;
    using NextA1 = typename B5Pass1::NextA1;

    static const uint modifiedPacks = B1Pass1::modifiedPacks | B2Pass1::modifiedPacks |
                                      B3Pass1::modifiedPacks | B4Pass1::modifiedPacks |
                                      B5Pass1::modifiedPacks;
};

template <typename B1, typename B2, typename B3, typename B4, typename B5>
template <typename Storage, typename A1>
template <typename A2>
struct Sequence_<B1, B2, B3, B4, B5>::Pass1<Storage, A1>::Pass2
{
    template <typename A3> struct Pass3;
    using B5Pass2  = typename B5Pass1::template Pass2<A2>;
    using B4Pass2  = typename B4Pass1::template Pass2<typename B5Pass2::NextA2>;
    using B3Pass2  = typename B3Pass1::template Pass2<typename B4Pass2::NextA2>;
    using B2Pass2  = typename B2Pass1::template Pass2<typename B3Pass2::NextA2>;
    using B1Pass2  = typename B1Pass1::template Pass2<typename B2Pass2::NextA2>;
    using NextA2 = typename B1Pass2::NextA2;
PRIVATE
    static const uint requiredVars6_ = 0u;
    static const uint requiredVars5_ = (requiredVars6_ | B5Pass2::requiredVars) & ~B5A1::wasModified;
    static const uint requiredVars4_ = (requiredVars5_ | B4Pass2::requiredVars) & ~B4A1::wasModified;
    static const uint requiredVars3_ = (requiredVars4_ | B3Pass2::requiredVars) & ~B3A1::wasModified;
    static const uint requiredVars2_ = (requiredVars3_ | B2Pass2::requiredVars) & ~B2A1::wasModified;
    static const uint requiredVars1_ = (requiredVars2_ | B1Pass2::requiredVars) & ~B1A1::wasModified;
PUBLIC
    static const uint requiredVars = requiredVars1_;
};

template <typename B1, typename B2, typename B3, typename B4, typename B5>
template <typename Storage, typename A1>
template <typename A2>
template <typename A3>
struct Sequence_<B1, B2, B3, B4, B5>::Pass1<Storage, A1>::Pass2<A2>::Pass3
{
    using ControlStructure = Sequence_<B1, B2, B3, B4, B5>;

    using B1Pass3 = typename B1Pass2::template Pass3<A3>;
    using B2Pass3 = typename B2Pass2::template Pass3<typename B1Pass3::NextA3>;
    using B3Pass3 = typename B3Pass2::template Pass3<typename B2Pass3::NextA3>;
    using B4Pass3 = typename B4Pass2::template Pass3<typename B3Pass3::NextA3>;
    using B5Pass3 = typename B5Pass2::template Pass3<typename B4Pass3::NextA3>;
    using NextA3  = typename B5Pass3::NextA3;

    using InAttributes   = Attributes<Storage, A1, typename B1Pass2::NextA2, A3>;
    using OutAttributes  = Attributes<Storage, typename B5Pass1::NextA1, A2, typename B5Pass3::NextA3>;

    B1Pass3 b1;
    B2Pass3 b2;
    B3Pass3 b3;
    B4Pass3 b4;
    B5Pass3 b5;

    static Pass3 create(Storage storage) {
        Pass3 result = INIT(Pass3);
        result.b1 = B1Pass3::create(storage);
        result.b2 = B2Pass3::create(storage);
        result.b3 = B3Pass3::create(storage);
        result.b4 = B4Pass3::create(storage);
        result.b5 = B5Pass3::create(storage);
        return result; 
    }

#ifndef __HLSL__
    static bool isVisibleInStream(PrintInfo const&);
    template <typename Stream>
    static void stream(Stream&, PrintInfo const&);
#endif
};

template <typename B1, typename B2, typename B3, typename B4, typename B5>
using Sequence5 = Sequence_<B1, B2, B3, B4, B5>;

template <typename B1, typename B2, typename B3, typename B4>
using Sequence4 = Sequence5<B1, B2, B3, B4, UnusedBlock_>;

template <typename B1, typename B2, typename B3>
using Sequence3 = Sequence4<B1, B2, B3, UnusedBlock_>;

template <typename B1, typename B2>
using Sequence2 = Sequence3<B1, B2, UnusedBlock_>;