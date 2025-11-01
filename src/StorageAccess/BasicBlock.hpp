#pragma once

#include "_Internal.hpp"

// Represents a basic block of code with access operations on variables.
// Template arguments are flags indicating which variables are accessed
// in which way:
//   read      = some or all code-paths may read the variable
//   mayWrite  = some code-paths may write to the variable
//   willWrite = all code-paths write to the variable
template <uint read, uint mayWrite, uint willWrite>
struct BasicBlock
{
    template <typename Storage, typename A1> struct Pass1;
    MAKE_ANNOTATABLE
};

template <uint read, uint mayWrite, uint willWrite>
template <typename Storage, typename A1>
struct BasicBlock<read, mayWrite, willWrite>::Pass1
{
PRIVATE
    using ModifyBlock = Modify_<Storage, A1, mayWrite | willWrite>;
PUBLIC
    template <typename A2> struct Pass2;
    static const uint modifiedPacks = Storage::pack(mayWrite | willWrite);
    using NextA1 = typename ModifyBlock::NextA1;
};

template <uint read, uint mayWrite, uint willWrite>
template <typename Storage, typename A1>
template <typename A2>
struct BasicBlock<read, mayWrite, willWrite>::Pass1<Storage, A1>::Pass2
{
    static const uint requiredVars = read | (mayWrite & ~willWrite);
PRIVATE
    using ModifyBlockPass2 = typename ModifyBlock::template Pass2<A2>;
    using LoadBlock = Load_<Storage, A1, typename ModifyBlockPass2::NextA2, requiredVars>;
PUBLIC
    template <typename A3> struct Pass3;
    using NextA2 = typename LoadBlock::NextA2;
};

template <uint read, uint mayWrite, uint willWrite>
template <typename Storage, typename A1>
template <typename A2>
template <typename A3>
struct BasicBlock<read, mayWrite, willWrite>::Pass1<Storage, A1>::Pass2<A2>::Pass3
{
PRIVATE
    using LoadBlockPass3 = typename LoadBlock::template Pass3<A3>;
    using ModifyBlockPass3 = typename ModifyBlockPass2::template Pass3<typename LoadBlockPass3::NextA3>;
    using MidAttributes = Attributes<Storage, A1, typename ModifyBlockPass2::NextA2, typename LoadBlockPass3::NextA3>;
PUBLIC
    using ControlStructure = BasicBlock<read, mayWrite, willWrite>;
    using NextA3 = typename ModifyBlockPass3::NextA3;
    using InAttributes = Attributes<Storage, A1, NextA2, A3>;
    using OutAttributes = Attributes<Storage, NextA1, A2, NextA3>;

    template <typename Container>
    void begin(inout(Container) container) {
	    LoadBlockPass3::call(storage_, container);
    }

    template <typename Container>
    void end(inout(Container) container) {
	    ModifyBlockPass3::call(storage_, container);
    }

    static Pass3 create(Storage storage) { 
        Pass3 result = INIT(Pass3);
        result.storage_ = storage;
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