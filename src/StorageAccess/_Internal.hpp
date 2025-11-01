#pragma once

#include "_Assert.hpp"
#include "StorageAccess.hpp"

#define MAKE_ANNOTATABLE                                                       \
PRIVATE                                                                        \
    template <typename Storage> struct DefaultTypes {                          \
        using Pass1 = Pass1<Storage, DefaultA1>;                               \
        using Pass2 = typename Pass1::template Pass2<DefaultA2>;               \
        using Pass3 = typename Pass2::template Pass3<DefaultA3>;               \
        using NA1 = typename Pass1::NextA1;                                    \
        using NA2 = typename Pass2::NextA2;                                    \
        using NA3 = typename Pass3::NextA3;                                    \
        TEMPLATE_ASSERT(AssertImplies, NA1::requestedStore, NA3::storedPacks); \
        TEMPLATE_ASSERT(AssertImplies, NA1::wasModified, NA1::requestedStore); \
    };                                                                         \
PUBLIC                                                                         \
    template <typename Storage>                                                \
    using Annotated = typename DefaultTypes<Storage>::Pass3;                   \
                                                                               \
    template <typename Storage>                                                \
    static Annotated<Storage> bind(Storage storage) {                          \
        return Annotated<Storage>::create(storage);                            \
    }

#ifndef __HLSL__
struct PrintInfo;
#endif

// Special type of empty block that is used to indicate unused slots in
// Sequence_ and Switch_
struct UnusedBlock_
{
    template <typename Storage, typename A1>
    struct Pass1
    {
        static const uint modifiedPacks = 0u;
        using NextA1 = A1;
        template <typename A2>
        struct Pass2
        {
            static const uint requiredVars = 0u;
            using NextA2 = A2;
            template <typename A3> struct Pass3
            {
                using NextA3 = A3;
                using InAttributes = Attributes<Storage, A1, A2, A3>;
                using OutAttributes = InAttributes;
                static Pass3 create(Storage storage) { return INIT(Pass3); }

#ifndef __HLSL__
                static bool isVisibleInStream(PrintInfo const&) { return false; }
                template <typename Stream>
                static void stream(Stream&, PrintInfo const&) { }
#endif
            };
        };
    };
};

// Makes all variables in "vars" available by loading their packs from storage if
// necessary. If the loaded packs contain additional variables that are read
// later on, This block makes these variables available as well.
template <typename Storage, typename A1, typename A2, uint vars>
struct Load_
{
        template <typename A3> struct Pass3;
    struct NextA2
    {
        static const uint isUsed = A2::isUsed | vars;
        static const uint blockStore = A2::blockStore;
    };
};


template <typename Storage, typename A1, typename A2, uint vars>
template <typename A3>
struct Load_<Storage, A1, A2, vars>::Pass3
{
PRIVATE
    static const uint loadedPacks_ = Storage::pack(vars & ~A3::isAvailable);
    static const uint loadedVars_ = ~A3::isAvailable & loadedPacks_ & NextA2::isUsed;
    // Special handling of store requests: In case a pack is loaded, we also
    // handle store requests for this pack if possible. Note that a pack
    // could not have been stored before, since then the full pack would
    // have been available. Also, it is not reasonable to delay the store
    // operation since the pack is fully available now.
    static const uint storedPacks_ = loadedPacks_ &
        Storage::pack(A1::requestedStore) & ~Storage::pack(A2::blockStore);
PUBLIC
    struct NextA3
    {
        static const uint isAvailable = A3::isAvailable | loadedVars_;
        static const uint loadedPacks = A3::loadedPacks | loadedPacks_;
        static const uint storedPacks = A3::storedPacks | storedPacks_;
    };
    using InAttributes = Attributes<Storage, A1, NextA2, A3>;
    using OutAttributes = Attributes<Storage, A1, A2, NextA3>;

    // Assert the all variables in "vars" are available after this block.
    TEMPLATE_ASSERT(AssertImplies, vars, NextA3::isAvailable);

    // One key feature of this project is avoiding doing a storage operation twice. By
    // asserting here, we make sure that this feature is supported correctly.
    TEMPLATE_ASSERT(AssertDisjoint, A3::loadedPacks, loadedPacks_);
    TEMPLATE_ASSERT(AssertDisjoint, A3::storedPacks, storedPacks_);

    // Makes "vars" available. Includes some special handling for packs for
    // which a store is requested.
    template <typename Container>
    static void call(Storage storage, inout(Container) container)
    {
        static const uint loadedVars = loadedVars_ | (~A3::isAvailable & storedPacks_);
        storage.load(container, loadedPacks_, loadedVars);
        storage.store(container, storedPacks_);
    };

    // Used by RequestStore_ to make "vars" available without handling store
    // requests, since they are processed by RequestStore_ directly.
    template <typename Container>
    static void callWithoutStore(Storage storage, inout(Container) container)
    {
        storage.load(container, loadedPacks_, loadedVars_);
    };

#ifndef __HLSL__
    static bool isVisibleInStream(PrintInfo const&, uint storedVars = storedPacks_);
    template <typename Stream>
    static void stream(Stream&, PrintInfo const&,
        std::string const& storeType = "delayed", uint storedVars = storedPacks_);
#endif
};

// Requests store operation for all variables in "vars". It ensture that the
// content of the variables is written to storage at some point. Note that store
// operations may be delayed until packs become available, to mitigate register
// pressure.
//
// This block must only be used at the beginning of a control structure or
// immediately after blocking previous stores of "vars". Otherwise, multiple
// stores may happen for the same pack.
template <typename Storage, typename A1, uint vars>
struct RequestStore_
{
    template <typename A2> struct Pass2;

    struct NextA1
    {
        static const uint requestedStore = A1::requestedStore | vars;
        static const uint wasModified = A1::wasModified; 
    };
};

template <typename Storage, typename A1, uint vars>
template <typename A2>
struct RequestStore_<Storage, A1, vars>::Pass2
{
PRIVATE
    static constexpr uint packsToStore() {
        uint v1 = Storage::pack(vars) & ~Storage::pack(A2::blockStore);
        uint v2 = Storage::template filterStoredPacks<A1, A2>(v1);
        return v2;
    }
    using LoadBlock = Load_<Storage, NextA1, A2, packsToStore()>;
PUBLIC
    template <typename A3> struct Pass3;
    using NextA2 = typename LoadBlock::NextA2;
};

template <typename Storage, typename A1, uint vars>
template <typename A2>
template <typename A3>
struct RequestStore_<Storage, A1, vars>::Pass2<A2>::Pass3
{
PRIVATE
    using LoadBlockPass3 = typename LoadBlock::template Pass3<A3>;
PUBLIC
    struct NextA3
    {
        static const uint isAvailable = LoadBlockPass3::NextA3::isAvailable;
        static const uint loadedPacks = LoadBlockPass3::NextA3::loadedPacks;
        // Do not copy the stored packs of LoadBlockPass3, since we call it
        // without storage operations.
        static const uint storedPacks = A3::storedPacks | packsToStore();
    };
    using InAttributes = Attributes<Storage, A1, NextA2, A3>;
    using OutAttributes = Attributes<Storage, NextA1, A2, NextA3>;

    // One key feature of this project is avoiding doing a storage operation twice. By
    // asserting here, we make sure that this feature is supported correctly.
    TEMPLATE_ASSERT(AssertDisjoint, A3::storedPacks, packsToStore());

    template <typename Container>
    static void call(Storage storage, inout(Container) container) {
        // RequestStore_ provides it own handling of store operations. Instead
        // of storing only the packs that were not available before (as done in
        // LoadBlockPass3::call()), we want to store _all_ packs.
        LoadBlockPass3::callWithoutStore(storage, container);
        storage.store(container, packsToStore());
    };

#ifndef __HLSL__
    static bool isVisibleInStream(PrintInfo const&);
    template <typename Stream>
    static void stream(Stream&, PrintInfo const&);
#endif
};

// Requests a store operation for all variables in "vars" and prevents stores
// from happening in any block preceeding the DelayStore block.
template <typename Storage, typename A1, uint vars>
struct DelayStore_
{
PRIVATE
    using RequestStoreBlock = RequestStore_<Storage, A1, vars>;
PUBLIC
    template <typename A2> struct Pass2;
    using NextA1 = typename RequestStoreBlock::NextA1;
};

template <typename Storage, typename A1, uint vars>
template <typename A2>
struct DelayStore_<Storage, A1, vars>::Pass2
{
PRIVATE
    using RequestStoreBlockPass2 = typename RequestStoreBlock::template Pass2<A2>;
PUBLIC
    template <typename A3> struct Pass3;
    struct NextA2
    {
        static const uint isUsed = RequestStoreBlockPass2::NextA2::isUsed;
        static const uint blockStore = RequestStoreBlockPass2::NextA2::blockStore | vars;
    };
};

template <typename Storage, typename A1, uint vars>
template <typename A2>
template <typename A3>
struct DelayStore_<Storage, A1, vars>::Pass2<A2>::Pass3
{
PRIVATE
    using RequestStoreBlockPass3 = typename RequestStoreBlockPass2::template Pass3<A3>;
PUBLIC
    using NextA3 = typename RequestStoreBlockPass3::NextA3;
    using InAttributes = Attributes<Storage, A1, NextA2, A3>;
    using OutAttributes = Attributes<Storage, NextA1, A2, NextA3>;

    template <typename Container>
    static void call(Storage storage, inout(Container) container)
    {
        RequestStoreBlockPass3::call(storage, container);
    };

#ifndef __HLSL__
    static bool isVisibleInStream(PrintInfo const&);
    template <typename Stream>
    static void stream(Stream&, PrintInfo const&);
#endif
};

// Handles a modification of variables, by marking them as available and
// delaying all store operation until the modification took place.
template <typename Storage, typename A1, uint vars>
struct Modify_
{
PRIVATE
    struct A1Tmp
    {
        static const uint requestedStore = A1::requestedStore;
        static const uint wasModified = A1::wasModified | vars;
    };
    using DelayStoreBlock = DelayStore_<Storage, A1Tmp, vars>;
PUBLIC
    template <typename A2> struct Pass2;
    using NextA1 = typename DelayStoreBlock::NextA1;
};

template <typename Storage, typename A1, uint vars>
template <typename A2>
struct Modify_<Storage, A1, vars>::Pass2
{
PRIVATE
    using DelayStoreBlockPass2 = typename DelayStoreBlock::template Pass2<A2>;
PUBLIC
    template <typename A3> struct Pass3;
    struct NextA2 
    {
        static const uint isUsed = DelayStoreBlockPass2::NextA2::isUsed & ~vars;
        static const uint blockStore = DelayStoreBlockPass2::NextA2::blockStore;
    };
};

template <typename Storage, typename A1, uint vars>
template <typename A2>
template <typename A3>
struct Modify_<Storage, A1, vars>::Pass2<A2>::Pass3
{
PRIVATE
    struct A3Tmp
    {
        static const uint isAvailable = A3::isAvailable | vars;
        static const uint loadedPacks = A3::loadedPacks;
        static const uint storedPacks = A3::storedPacks;
    };
    using DelayStoreBlockPass3 = typename DelayStoreBlockPass2::template Pass3<A3Tmp>;
PUBLIC
    using NextA3 = typename DelayStoreBlockPass3::NextA3;
    using InAttributes = Attributes<Storage, A1, NextA2, A3>;
    using OutAttributes = Attributes<Storage, NextA1, A2, NextA3>;

    template <typename Container>
    static void call(Storage storage, inout(Container) container)
    {
        DelayStoreBlockPass3::call(storage, container);
    };

#ifndef __HLSL__
    static bool isVisibleInStream(PrintInfo const&);
    template <typename Stream>
    static void stream(Stream&, PrintInfo const&);
#endif
};

// Logic for counting flags

template <typename X, typename Counts>
struct AddOne_ {
    using F = typename Counts::F;
PRIVATE
	static const uint value_ = F::template eval<X>();
PUBLIC
	static const uint oneOrMore = Counts::oneOrMore | value_;
	static const uint twoOrMore = Counts::twoOrMore | (value_ & Counts::oneOrMore);
	static const uint all = Counts::all & value_;
};

// Skip unused blocks
template <typename Counts>
struct AddOne_<UnusedBlock_, Counts> {
    using F = typename Counts::F;
	static const uint oneOrMore = Counts::oneOrMore;
	static const uint twoOrMore = Counts::twoOrMore;
	static const uint all = Counts::all;;
};

template <typename EvalFn>
struct Zero_ {
    using F = EvalFn;
	static const uint oneOrMore = 0u;
	static const uint twoOrMore = 0u;
	static const uint all = ~0u;
};

// Counts the number of flags returned by a meta function F invoked on various
// types. We differentiate between flags occuring at least once, at least twice,
// or every time in F(X1), F(X2), F(X3), ...
template <typename F, typename X1, typename X2, typename X3, typename X4, typename X5>
using Count5_ = AddOne_<X1, AddOne_<X2, AddOne_<X3, AddOne_<X4, AddOne_<X5, Zero_<F> > > > > >;

// Meta functions for counting attributes

template <typename Storage, typename A1>
struct ModifiedPacksAttr
{
    template <typename X>
    static constexpr uint eval() {
        using Pass = typename X::template Pass1<Storage, A1>;
        return Pass::modifiedPacks;
    }
};

template <typename Storage, typename A1, typename A2>
struct RequiredVarsAttr
{
    template <typename X>
    static constexpr uint eval() {
        using Pass = typename X::template Pass1<Storage, A1>::template Pass2<A2>;
        return Pass::requiredVars;
    }
};

template <typename Storage, typename A1, typename A2>
struct RequiredPacksAttr
{
    template <typename X>
    static constexpr uint eval() {
        using Pass = typename X::template Pass1<Storage, A1>::template Pass2<A2>;
        return Storage::pack(Pass::requiredVars);
    }
};

template <typename Storage, typename A1>
struct NextRequestedStoreAttr
{
    template <typename X>
    static constexpr uint eval() {
        using Pass = typename X::template Pass1<Storage, A1>;
        return Pass::NextA1::requestedStore;
    }
};

template <typename Storage, typename A1>
struct NextWasModifiedAttr
{
    template <typename X>
    static constexpr uint eval() {
        using Pass = typename X::template Pass1<Storage, A1>;
        return Pass::NextA1::wasModified;
    }
};

template <typename Storage, typename A1, typename A2>
struct NextIsUsedAttr
{
    template <typename X>
    static constexpr uint eval() {
        using Pass = typename X::template Pass1<Storage, A1>::template Pass2<A2>;
        return Pass::NextA2::isUsed;
    }
};

template <typename Storage, typename A1, typename A2>
struct NextBlockStoreAttr
{
    template <typename X>
    static constexpr uint eval() {
        using Pass = typename X::template Pass1<Storage, A1>::template Pass2<A2>;
        return Pass::NextA2::blockStore;
    }
};

template <typename Storage, typename A1, typename A2, typename A3>
struct NextIsAvailableAttr
{
    template <typename X>
    static constexpr uint eval() {
        using Pass = typename X::template Pass1<Storage, A1>::template Pass2<A2>::template Pass3<A3>;
        return Pass::NextA3::isAvailable;
    }
};

template <typename Storage, typename A1, typename A2, typename A3>
struct NextLoadedPacksAttr
{
    template <typename X>
    static constexpr uint eval() {
        using Pass = typename X::template Pass1<Storage, A1>::template Pass2<A2>::template Pass3<A3>;
        return Pass::NextA3::loadedPacks;
    }
};

template <typename Storage, typename A1, typename A2, typename A3>
struct NextStoredPacksAttr
{
    template <typename X>
    static constexpr uint eval() {
        using Pass = typename X::template Pass1<Storage, A1>::template Pass2<A2>::template Pass3<A3>;
        return Pass::NextA3::storedPacks;
    }
};

// Contains the correct attribute transformations for branching into one of the
// provided case blocks.

template <typename CountsProvider, typename Storage, typename A1>
struct Cases_
{
PRIVATE
    template <typename F>
    using Counts = typename CountsProvider::template Get<F>;
    using RequestedStoreCounts = Counts<NextRequestedStoreAttr<Storage, A1> >;
    using WasModifiedCounts = Counts<NextWasModifiedAttr<Storage, A1> >;
PUBLIC
    template <typename A2> struct Pass2;
    struct NextA1
    {
        static const uint requestedStore = RequestedStoreCounts::oneOrMore;
        static const uint wasModified = WasModifiedCounts::oneOrMore;
    };
};

template <typename CountsProvider, typename Storage, typename A1>
template <typename A2>
struct Cases_<CountsProvider, Storage, A1>::Pass2
{
PRIVATE
    using IsUsedCounts = Counts<NextIsUsedAttr<Storage, A1, A2> >;
    using BlockStoreCounts = Counts<NextBlockStoreAttr<Storage, A1, A2> >;
PUBLIC
    template <typename A3> struct Pass3;
    struct NextA2
    {
        static const uint isUsed = IsUsedCounts::oneOrMore;
        static const uint blockStore = BlockStoreCounts::oneOrMore;
    };
};

template <typename CountsProvider, typename Storage, typename A1>
template <typename A2>
template <typename A3>
struct Cases_<CountsProvider, Storage, A1>::Pass2<A2>::Pass3
{
PRIVATE
    using IsAvailableCounts = Counts<NextIsAvailableAttr<Storage, A1, A2, A3> >;
    using LoadedPacksCounts = Counts<NextLoadedPacksAttr<Storage, A1, A2, A3> >;
    using StoredPacksCount = Counts<NextStoredPacksAttr<Storage, A1, A2, A3> >;
PUBLIC
    struct NextA3
    {
        static const uint isAvailable = IsAvailableCounts::all;
        static const uint loadedPacks = LoadedPacksCounts::oneOrMore;
        static const uint storedPacks = StoredPacksCount::oneOrMore;
    };

    // Assert that all variables modified in at least one case also are
    // available at the end of each case. Otherwise the implication
    // "A1::wasModified -> A3::isAvailable" would be violated. The switch block
    // that makes use of Cases_ is responsible for loading variables early such
    // that this assertion is not tripped.
    TEMPLATE_ASSERT(AssertImplies, NextA1::wasModified & A2::isUsed, NextA3::isAvailable);

    // Having the same storage operation in more than one branch, may result in
    // redundant storage access in case lanes of a wave diverge. One key feature
    // of this project is avoiding these redundant accesses. By asserting here,
    // we make sure that this feature is supported correctly.
    TEMPLATE_ASSERT(AssertImplies, LoadedPacksCounts::twoOrMore, A3::loadedPacks);
    TEMPLATE_ASSERT(AssertImplies, StoredPacksCount::twoOrMore, A3::storedPacks);
};