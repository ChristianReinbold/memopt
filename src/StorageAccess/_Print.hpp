#pragma once

#include <Kernel.hpp>

#include "_Internal.hpp"
#include "StorageAccess.hpp"

#ifndef __HLSL__

enum PrintFlags : uint
{
    Pass1      = 1 << 0,
    Pass2      = 1 << 1,
    Pass3      = 1 << 2,
    AccAttrs   = 1 << 3,
    StorageOp  = 1 << 4,
    Internal   = 1 << 5,
    BlockArgs  = 1 << 6,
    XML        = 1 << 7,

    AllPasses = Pass1 | Pass2 | Pass3,
    Verbose = AllPasses | StorageOp | Internal | BlockArgs | AccAttrs,
    User = StorageOp | BlockArgs
};

struct PrintInfo
{
    uint flags() const { return flags_; }
    std::string indent() const { return current_; }
    bool enclosedAttrsPrinted() const { return enclose_; }
    std::string indentStorageOp() const {
        if (flags_ & PrintFlags::XML) {
            return current_ + (flags_ & PrintFlags::Internal ? "    " : "");
        } else {
            return current_ + (flags_ & PrintFlags::Internal ? "|-" : "");
        }
    }

    PrintInfo(uint flags) : flags_(flags), current_(""), enclose_(true) { }

    PrintInfo nest() const {
        PrintInfo result(flags_);
        result.current_ = current_ + "    ";
        result.enclose_ = false;
        return result;
    }

    PrintInfo removeFlags(uint flags) const {
        PrintInfo result(flags_ & ~flags);
        result.current_ = current_ ;
        result.enclose_ = enclose_;
        return result;
    }

    PrintInfo removeEnclosed() const {
        PrintInfo result(flags_);
        result.current_ = current_;
        result.enclose_ = false;
        return result;
    }

    PrintInfo nestWithEnclosedAttrs() const {
        PrintInfo result = nest();
        result.enclose_ = true;
        return result;
    }
private:
    uint flags_;
    std::string current_;
    bool enclose_;
};

template <typename Storage, typename A1, typename A2, typename A3>
bool Attributes<Storage, A1, A2, A3>::isVisibleInStream(PrintInfo const& info) {
    return info.flags() & (PrintFlags::AllPasses | PrintFlags::AccAttrs);
}

template <typename Storage, typename A1, typename A2, typename A3>
template <typename Stream>
void Attributes<Storage, A1, A2, A3>::stream(Stream& stream, PrintInfo const& info) {
    if (!isVisibleInStream(info)) {
        return;
    }
    uint flags = info.flags();
    bool isXml = flags & PrintFlags::XML;
    stream << info.indent();
    if (isXml) {
        stream << "<!--";
    } else {
        stream << "#";
    }

    if (flags & PrintFlags::Pass3) {
        stream << " avail=[" << typename Storage::Var(isAvailable) << "]";
    }
    if (flags & PrintFlags::Pass1) {
        stream << " mod=[" << typename Storage::Var(wasModified) << "]";
    }
    if (flags & PrintFlags::Pass2) {
        stream << " used=[" << typename Storage::Var(isUsed) << "]";
    }
    if (flags & PrintFlags::Pass1) {
        stream << " req_store=[" << typename Storage::Var(requestedStore) << "]";
    }
    if (flags & PrintFlags::Pass2) {
        stream << " block_store=[" << typename Storage::Var(blockStore) << "]";
    }
    if (flags & PrintFlags::AccAttrs) {
        stream << " loaded=[" << typename Storage::Pack(loadedPacks) << "]";
        stream << " stored=[" << typename Storage::Pack(storedPacks) << "]";
    }
    if (isXml) {
        stream << " -->";
    }
    stream << " \n";
}


template <typename Pass3, typename Stream>
void streamEnclosedAttrBegin(Stream& stream, PrintInfo const& info) {
    if (!info.enclosedAttrsPrinted()) {
        return;
    }
    Pass3::InAttributes::stream(stream, info.removeFlags(PrintFlags::Pass1 | PrintFlags::Pass3 | PrintFlags::AccAttrs));
}

template <typename Pass3, typename Stream>
void streamEnclosedAttrEnd(Stream& stream, PrintInfo const& info) {
    if (!info.enclosedAttrsPrinted()) {
        return;
    }
    
    Pass3::OutAttributes::stream(stream, info.removeFlags(PrintFlags::Pass2));
}

template <typename Storage, typename A1, typename A2, uint vars>
template <typename A3>
bool Load_<Storage, A1, A2, vars>::Pass3<A3>::isVisibleInStream(PrintInfo const& info, uint storedVars)
{
    bool hasChildren = (info.flags() & PrintFlags::StorageOp) && (loadedPacks_ | loadedVars_ | storedVars);
    bool hasHead = info.flags() & PrintFlags::Internal;
    return vars != 0 && (hasHead || hasChildren);
}

template <typename Storage, typename A1, typename A2, uint vars>
template <typename A3>
template <typename Stream>
void Load_<Storage, A1, A2, vars>::Pass3<A3>::stream(
    Stream& stream, PrintInfo const& info,
    std::string const& storeType, uint storedVars)
{
    if (!isVisibleInStream(info, storedVars)) {
        return;
    }

    auto flags = info.flags();
    bool isXml = flags & PrintFlags::XML;
    bool hasChildren = (flags & PrintFlags::StorageOp) && (loadedPacks_ | loadedVars_ | storedVars);
    bool streamInternal = flags & PrintFlags::Internal;
    if (streamInternal) {
        stream << info.indent();
        if (isXml) stream << "<";
        stream << "LoadStore";
        if (flags & PrintFlags::BlockArgs) {
            stream << (isXml ? " vars=\"" : "<");
            stream << typename Storage::Var(vars);
            stream << (isXml ? "\"" : ">");
        }
        if (isXml) stream << (hasChildren ? ">" : "/>");
        stream << "\n";
    }
    if (flags & PrintFlags::StorageOp) {
        if (loadedPacks_) {
            stream << info.indentStorageOp();
            stream << (isXml ? "<load>" : "load [");
            stream << typename Storage::Pack(loadedPacks_);
            stream << (isXml ? "</load>\n" : "]\n");
        }
        if (loadedVars_) {
            stream << info.indentStorageOp();
            stream << (isXml ? "<set>" : "set [");
            stream << typename Storage::Var(loadedVars_);
            stream << (isXml ? "</set>\n" : "]\n");
        }
        if (storedVars) {
            stream << info.indentStorageOp();
            if (isXml) {
                stream << "<store sync=\"" << storeType << "\">";
            } else {
                stream << "store [";
            }
            stream << typename Storage::Var(storedVars);
            if (isXml) {
                stream << "</store>\n";
            } else {
                stream << "] (" << storeType << " sync)\n";
            }
        }
    }
    if (streamInternal && isXml && hasChildren) stream << info.indent() << "</LoadStore>\n";
}

template <typename Storage, typename A1, uint vars>
template <typename A2>
template <typename A3>
bool RequestStore_<Storage, A1, vars>::Pass2<A2>::Pass3<A3>::isVisibleInStream(
    PrintInfo const& info)
{
    return LoadBlockPass3::isVisibleInStream(info, packsToStore());
}

template <typename Storage, typename A1, uint vars>
template <typename A2>
template <typename A3>
template <typename Stream>
void RequestStore_<Storage, A1, vars>::Pass2<A2>::Pass3<A3>::stream(
    Stream& stream, PrintInfo const& info)
{
    if (!isVisibleInStream(info)) {
        return;
    }

    auto flags = info.flags();
    bool isXml = flags & PrintFlags::XML;
    if (flags & PrintFlags::Internal) {
        stream << info.indent();
        if (isXml) stream << "<";
        stream << "RequestStore";
        if (flags & PrintFlags::BlockArgs) {
            stream << (isXml ? " vars=\"" : "<");
            stream << typename Storage::Var(vars);
            stream << (isXml ? "\"" : ">");
        }
        if (isXml) stream << "/>";
        stream << "\n";
    }
    if (LoadBlockPass3::isVisibleInStream(info)) {
        LoadBlockPass3::InAttributes::stream(stream, info);
    }
    LoadBlockPass3::stream(stream, info, "immediate", packsToStore());
}

template <typename Storage, typename A1, uint vars>
template <typename A2>
template <typename A3>
bool DelayStore_<Storage, A1, vars>::Pass2<A2>::Pass3<A3>::isVisibleInStream(
    PrintInfo const& info)
{
    return RequestStoreBlockPass3::isVisibleInStream(info);
}

template <typename Storage, typename A1, uint vars>
template <typename A2>
template <typename A3>
template <typename Stream>
void DelayStore_<Storage, A1, vars>::Pass2<A2>::Pass3<A3>::stream(
    Stream& stream, PrintInfo const& info)
{
    if (!isVisibleInStream(info)) {
        return;
    }

    auto flags = info.flags();
    bool isXml = flags & PrintFlags::XML;
    if (flags & PrintFlags::Internal) {
        stream << info.indent();
        if (isXml) stream << "<";
        stream << "DelayStore";
        if (flags & PrintFlags::BlockArgs) {
            stream << (isXml ? " vars=\"" : "<");
            stream << typename Storage::Var(vars);
            stream << (isXml ? "\"" : ">");
        }
        if (isXml) stream << "/>";
        stream << "\n";
    }
    RequestStoreBlockPass3::InAttributes::stream(stream, info);
    RequestStoreBlockPass3::stream(stream, info);
}

template <typename Storage, typename A1, uint vars>
template <typename A2>
template <typename A3>
bool Modify_<Storage, A1, vars>::Pass2<A2>::Pass3<A3>::isVisibleInStream(
    PrintInfo const& info)
{
    return DelayStoreBlockPass3::isVisibleInStream(info);
}

template <typename Storage, typename A1, uint vars>
template <typename A2>
template <typename A3>
template <typename Stream>
void Modify_<Storage, A1, vars>::Pass2<A2>::Pass3<A3>::stream(
    Stream& stream, PrintInfo const& info)
{
    if (!isVisibleInStream(info)) {
        return;
    }

    auto flags = info.flags();
    bool isXml = flags & PrintFlags::XML;
    if (flags & PrintFlags::Internal) {
        stream << info.indent();
        if (isXml) stream << "<";
        stream << "Modify";
        if (flags & PrintFlags::BlockArgs) {
            stream << (isXml ? " vars=\"" : "<");
            stream << typename Storage::Var(vars);
            stream << (isXml ? "\"" : ">");
        }
        if (isXml) stream << "/>";
        stream << "\n";
    }
    DelayStoreBlockPass3::InAttributes::stream(stream, info);
    DelayStoreBlockPass3::stream(stream, info);
}

template <typename Storage, typename A1>
template <typename A2>
template <typename A3>
bool EmptyBlock::Pass1<Storage, A1>::Pass2<A2>::Pass3<A3>::isVisibleInStream(
    PrintInfo const& info)
{
    return true;
}

template <typename Storage, typename A1>
template <typename A2>
template <typename A3>
template <typename Stream>
void EmptyBlock::Pass1<Storage, A1>::Pass2<A2>::Pass3<A3>::stream(
    Stream& stream, PrintInfo const& info)
{
    bool isXml = info.flags() & PrintFlags::XML;
    streamEnclosedAttrBegin<Pass3>(stream, info);
    stream << info.indent() << (isXml ? "<EmptyBlock />" : "EmptyBlock") << "\n";
    streamEnclosedAttrEnd<Pass3>(stream, info);
}

template <uint read, uint mayWrite, uint willWrite>
template <typename Storage, typename A1>
template <typename A2>
template <typename A3>
bool BasicBlock<read, mayWrite, willWrite>::Pass1<Storage, A1>::Pass2<A2>::Pass3<A3>::isVisibleInStream(
    PrintInfo const& info)
{
    return true;
}

template <typename VarOrPack, typename Stream>
void streamBlockArg(Stream& stream, PrintInfo const& info,
    std::string const& name, uint value, bool addDelim)
{
    bool isXml = info.flags() & PrintFlags::XML;
    if (addDelim) {
        stream << (isXml ? " " : ", ");
    }
    stream << name << "=";
    stream << (isXml ? "\"" : "[");
    stream << VarOrPack(value);
    stream << (isXml ? "\"" : "]");
}

template <uint read, uint mayWrite, uint willWrite>
template <typename Storage, typename A1>
template <typename A2>
template <typename A3>
template <typename Stream>
void BasicBlock<read, mayWrite, willWrite>::Pass1<Storage, A1>::Pass2<A2>::Pass3<A3>::stream(
    Stream& stream, PrintInfo const& info)
{
    uint rVars = read & ~willWrite;
    uint wVars = willWrite & ~read;
    uint rwVars = mayWrite | (read & willWrite);

    auto flags = info.flags();
    auto childInfo = info.nest();
    bool isXml = flags & PrintFlags::XML;
    bool hasChildren = LoadBlockPass3::isVisibleInStream(childInfo) |
        ModifyBlockPass3::isVisibleInStream(childInfo);
    streamEnclosedAttrBegin<Pass3>(stream, info);
    stream << info.indent();
    if (isXml) stream << "<";
    stream << "BasicBlock";
    if (flags & PrintFlags::BlockArgs) {
        stream << (isXml ? " " : "<");
        streamBlockArg<typename Storage::Var>(stream, info, "r", rVars, false);
        streamBlockArg<typename Storage::Var>(stream, info, "w", wVars, true);
        streamBlockArg<typename Storage::Var>(stream, info, "rw", rwVars, true);
        stream << (isXml ? "" : ">");
    }
    if (isXml) stream << (hasChildren ? ">" : "/>");
    stream << "\n";
    LoadBlockPass3::stream(stream, childInfo);
    if (LoadBlockPass3::isVisibleInStream(childInfo) && ModifyBlockPass3::isVisibleInStream(childInfo)) {
        ModifyBlockPass3::InAttributes::stream(stream, childInfo);
    }
    ModifyBlockPass3::stream(stream, childInfo);
    if (isXml && hasChildren) stream << info.indent() << "</BasicBlock>\n";
    streamEnclosedAttrEnd<Pass3>(stream, info);
}

template <typename B1, typename B2, typename B3, typename B4, typename B5>
template <typename Storage, typename A1>
template <typename A2>
template <typename A3>
bool Sequence_<B1, B2, B3, B4, B5>::Pass1<Storage, A1>::Pass2<A2>::Pass3<
    A3>::isVisibleInStream(PrintInfo const &info) 
{
    return B1Pass3::isVisibleInStream(info) ||
        B2Pass3::isVisibleInStream(info) ||
        B3Pass3::isVisibleInStream(info) ||
        B4Pass3::isVisibleInStream(info) ||
        B5Pass3::isVisibleInStream(info);
}

template <typename B1, typename B2, typename B3, typename B4, typename B5>
template <typename Storage, typename A1>
template <typename A2>
template <typename A3>
template <typename Stream>
void Sequence_<B1, B2, B3, B4, B5>::Pass1<Storage, A1>::Pass2<A2>::Pass3<
    A3>::stream(Stream& stream, PrintInfo const& info) 
{
    if (!isVisibleInStream(info))
    {
        return;
    }

    streamEnclosedAttrBegin<Pass3>(stream, info);
    auto childInfo = info.removeEnclosed();
    bool visible1 = B1Pass3::isVisibleInStream(childInfo);
    bool visible2 = B2Pass3::isVisibleInStream(childInfo);
    bool visible3 = B3Pass3::isVisibleInStream(childInfo);
    bool visible4 = B4Pass3::isVisibleInStream(childInfo);
    bool visible5 = B5Pass3::isVisibleInStream(childInfo);
    bool anyVisible = false;
    B1Pass3::stream(stream, childInfo);
    anyVisible |= visible1;
    if (anyVisible && visible2) B1Pass3::OutAttributes::stream(stream, childInfo);
    B2Pass3::stream(stream, childInfo);
    anyVisible |= visible2;
    if (anyVisible && visible3) B2Pass3::OutAttributes::stream(stream, childInfo);
    B3Pass3::stream(stream, childInfo);
    anyVisible |= visible3;
    if (anyVisible && visible4) B3Pass3::OutAttributes::stream(stream, childInfo);
    B4Pass3::stream(stream, childInfo);
    anyVisible |= visible4;
    if (anyVisible && visible5) B4Pass3::OutAttributes::stream(stream, childInfo);
    B5Pass3::stream(stream, childInfo);
    streamEnclosedAttrEnd<Pass3>(stream, info);
}

template <typename C1, typename C2, typename C3, typename C4, typename C5>
template <typename Storage, typename A1>
template <typename A2>
template <typename A3>
bool Switch_<C1, C2, C3, C4, C5>::Pass1<Storage, A1>::Pass2<A2>::Pass3<
    A3>::isVisibleInStream(PrintInfo const& info)
{
    return true;
}

template <typename C1, typename C2, typename C3, typename C4, typename C5>
template <typename Storage, typename A1>
template <typename A2>
template <typename A3>
template <typename Stream>
void Switch_<C1, C2, C3, C4, C5>::Pass1<Storage, A1>::Pass2<A2>::Pass3<
    A3>::stream(Stream& stream, PrintInfo const& info)
{
    auto flags = info.flags();
    auto childInfo = info.nest();
    auto caseInfo = childInfo.nestWithEnclosedAttrs();
    bool isXml = flags & PrintFlags::XML;
    streamEnclosedAttrBegin<Pass3>(stream, info);
    stream << info.indent();
    if (isXml) stream << "<";
    stream << "Switch";
    if (flags & PrintFlags::BlockArgs) {
        stream << (isXml ? " " : "<");
        streamBlockArg<typename Storage::Var>(stream, info, "load_early", loadEarly(), false);
        streamBlockArg<typename Storage::Var>(stream, info, "store_late", storeLate(), true);
        stream << (isXml ? "" : ">");
    }
    if (isXml) stream << ">";
    stream << "\n";
    LoadBlockPass3::stream(stream, childInfo);
    LoadBlockPass3::OutAttributes::stream(stream, childInfo);
    if (Case1Pass3::isVisibleInStream(childInfo)) {
        stream << childInfo.indent() << (isXml ? "<Case>" : "Case:") << "\n";
        Case1Pass3::stream(stream, caseInfo);
        if (isXml) stream << childInfo.indent() << "</Case>\n";
    }
    if (Case2Pass3::isVisibleInStream(childInfo)) {
        stream << childInfo.indent() << (isXml ? "<Case>" : "Case:") << "\n";
        Case2Pass3::stream(stream, caseInfo);
        if (isXml) stream << childInfo.indent() << "</Case>\n";
    }
    if (Case3Pass3::isVisibleInStream(childInfo)) {
        stream << childInfo.indent() << (isXml ? "<Case>" : "Case:") << "\n";
        Case3Pass3::stream(stream, caseInfo);
        if (isXml) stream << childInfo.indent() << "</Case>\n";
    }
    if (Case4Pass3::isVisibleInStream(childInfo)) {
        stream << childInfo.indent() << (isXml ? "<Case>" : "Case:") << "\n";
        Case4Pass3::stream(stream, caseInfo);
        if (isXml) stream << childInfo.indent() << "</Case>\n";
    }
    if (Case5Pass3::isVisibleInStream(childInfo)) {
        stream << childInfo.indent() << (isXml ? "<Case>" : "Case:") << "\n";
        Case5Pass3::stream(stream, caseInfo);
        if (isXml) stream << childInfo.indent() << "</Case>\n";
    }
    DelayStoreBlockPass3::InAttributes::stream(stream, childInfo);
    DelayStoreBlockPass3::stream(stream, childInfo);
    if (isXml) stream << info.indent() << "</Switch>\n";
    streamEnclosedAttrEnd<Pass3>(stream, info);
}

#endif