#pragma once

#include "_Internal.hpp"
#include "StorageAccess.hpp"

// TODO: Comment
template <typename C1, typename C2, typename C3, typename C4, typename C5>
struct Switch_
{
PRIVATE
    template <typename F>
    using Counts = Count5_<F, C1, C2, C3, C4, C5>;
PUBLIC
    template <typename Storage, typename A1> struct Pass1;
    MAKE_ANNOTATABLE
};

template <typename C1, typename C2, typename C3, typename C4, typename C5>
template <typename Storage, typename A1>
struct Switch_<C1, C2, C3, C4, C5>::Pass1
{
PRIVATE
    struct CountsProvider { template <typename F> using Get = Counts<F>; };
    using CasesBlock = Cases_<CountsProvider, Storage, A1>;
    using CaseA1 = A1;

    using ModifiedPacksCounts = Counts<ModifiedPacksAttr<Storage, A1> >;
    static constexpr uint storeLate() {
        // If a pack is modified by two cases, we delay to prevent redudant storage ops.
        uint redundant = ModifiedPacksCounts::twoOrMore;
        // If a store is requested from the outside, we cannot satisfy them in a
        // branched code-path. Hence we block those stores as well.
        uint requested = Storage::pack(A1::requestedStore);
        // We only want to delay stores for those variables that are modified in
        // some cases. In that case, stores in branches may happen which we need
        // to prevent.
        // TODO: "modified" may not be enough. Stores can also happen if a
        // branch completes a pack of A1::requestedStore. However, we certainly
        // do not want to block all A1::requestedStore, since this will prevent
        // ANY stores before a switch-block.
        // TODO: Sample scenario:
        // Store of pack is delayed from earlier, and then will be loaded in one case
        // => This case will do a delayed store (which is not what we want!)
        // => We should rather have loaded early and process the store there.
        // Extra case in loadEarly(): Storage::pack(RequiredVarsCounts::OneOrMore) & Storage::pack(A1::requestedStore)
        uint touched = ModifiedPacksCounts::oneOrMore;
        return (redundant | requested) & touched;
    }

    using DelayStoreBlock = DelayStore_<Storage, typename CasesBlock::NextA1, storeLate()>;
PUBLIC
    using NextA1 = typename DelayStoreBlock::NextA1;
    static const uint modifiedPacks = ModifiedPacksCounts::oneOrMore;
    template <typename A2> struct Pass2;
};

template <typename C1, typename C2, typename C3, typename C4, typename C5>
template <typename Storage, typename A1>
template <typename A2>
struct Switch_<C1, C2, C3, C4, C5>::Pass1<Storage, A1>::Pass2
{
PRIVATE
    using DelayStoreBlockPass2 = typename DelayStoreBlock::template Pass2<A2>;
    using CaseA2 = typename DelayStoreBlockPass2::NextA2;
    using CasesBlockPass2 = typename CasesBlock::template Pass2<CaseA2>;

    using RequiredVarsCounts = Counts<RequiredVarsAttr<Storage, A1, A2> >;
    using RequiredPacksCounts = Counts<RequiredPacksAttr<Storage, A1, A2> >;
    static constexpr uint loadEarly() {
        uint guaranteedModification = A1::wasModified | ModifiedPacksCounts::all;
        uint requiredAfterMerge = ~guaranteedModification & CaseA2::isUsed;
        uint preventDoubleLoads = RequiredPacksCounts::twoOrMore | (RequiredPacksCounts::oneOrMore & Storage::pack(requiredAfterMerge));
        uint preventOverwrite = requiredAfterMerge & ModifiedPacksCounts::oneOrMore;
        // TODO: Check if this behaves as intended.
        uint preventPartialDelayedStores = RequiredPacksCounts::oneOrMore & Storage::pack(A1::requestedStore) & ~Storage::pack(CaseA2::blockStore);
        uint isUsed = CasesBlockPass2::NextA2::isUsed;
        return (preventDoubleLoads | preventOverwrite | preventPartialDelayedStores) & isUsed;
    }

    using LoadBlock = Load_<Storage, A1, typename CasesBlockPass2::NextA2, loadEarly()>;
PUBLIC
    using NextA2 = typename LoadBlock::NextA2;
    template <typename A3> struct Pass3;
    static const uint requiredVars = RequiredVarsCounts::oneOrMore;
};

template <typename C1, typename C2, typename C3, typename C4, typename C5>
template <typename Storage, typename A1>
template <typename A2>
template <typename A3>
struct Switch_<C1, C2, C3, C4, C5>::Pass1<Storage, A1>::Pass2<A2>::Pass3
{
PRIVATE
    using LoadBlockPass3 = typename LoadBlock::template Pass3<A3>;
    using CaseA3 = typename LoadBlockPass3::NextA3;
    using CasesBlockPass3 = typename CasesBlockPass2::template Pass3<CaseA3>;
    using DelayStoreBlockPass3 = typename DelayStoreBlockPass2::template Pass3<typename CasesBlockPass3::NextA3>;

    using Case1Pass3 = typename C1::template Pass1<Storage, CaseA1>::template Pass2<CaseA2>::template Pass3<CaseA3>;
    using Case2Pass3 = typename C2::template Pass1<Storage, CaseA1>::template Pass2<CaseA2>::template Pass3<CaseA3>;
    using Case3Pass3 = typename C3::template Pass1<Storage, CaseA1>::template Pass2<CaseA2>::template Pass3<CaseA3>;
    using Case4Pass3 = typename C4::template Pass1<Storage, CaseA1>::template Pass2<CaseA2>::template Pass3<CaseA3>;
    using Case5Pass3 = typename C5::template Pass1<Storage, CaseA1>::template Pass2<CaseA2>::template Pass3<CaseA3>;
PUBLIC
    using NextA3 = typename DelayStoreBlockPass3::NextA3;
    using InAttributes = Attributes<Storage, A1, NextA2, A3>;
    using OutAttributes = Attributes<Storage, NextA1, A2, NextA3>;
    using ControlStructure = Switch_<C1, C2, C3, C4, C5>;

    template <typename Container>
    void begin(inout(Container) container) {
	    LoadBlockPass3::call(storage_, container);
    }

    template <typename Container>
    void end(inout(Container) container) {
	    DelayStoreBlockPass3::call(storage_, container);
    }

    Case1Pass3 c1;
    Case2Pass3 c2;
    Case3Pass3 c3;
    Case4Pass3 c4;
    Case5Pass3 c5;

    static Pass3 create(Storage storage) {
        Pass3 result = INIT(Pass3);
        result.storage_ = storage;
        result.c1 = Case1Pass3::create(storage);
        result.c2 = Case2Pass3::create(storage);
        result.c3 = Case3Pass3::create(storage);
        result.c4 = Case4Pass3::create(storage);
        result.c5 = Case5Pass3::create(storage);
        return result; 
    }

#ifndef __HLSL__
    static bool isVisibleInStream(PrintInfo const&);
    template <typename Stream>
    static void stream(Stream&, PrintInfo const&);
#endif

PRIVATE
    Storage storage_;
};

template <typename B1, typename B2, typename B3, typename B4, typename B5>
using Switch5 = Switch_<B1, B2, B3, B4, B5>;

template <typename B1, typename B2, typename B3, typename B4>
using Switch4 = Switch5<B1, B2, B3, B4, UnusedBlock_>;

template <typename B1, typename B2, typename B3>
using Switch3 = Switch4<B1, B2, B3, UnusedBlock_>;

template <typename B1, typename B2>
using Switch2 = Switch3<B1, B2, UnusedBlock_>;
