#pragma once

#include <Kernel.hpp>
#include <StorageBase.hpp>

[[vk::binding(0,0)]] RWStructuredBuffer<uint> a;

void Storage::load(inout(Vars) dst, uint loadedPacks, uint loadedVars) {
    if(loadedVars & Var::A1) dst.a1 = a[0];
    if(loadedVars & Var::A2) dst.a2 = a[1];
    if(loadedVars & Var::B ) dst.b  = a[2];
    if(loadedVars & Var::C1) dst.c1 = a[3];
    if(loadedVars & Var::C2) dst.c2 = a[4];
    if(loadedVars & Var::D)  dst.d  = a[5];
    if(loadedVars & Var::E)  dst.e  = a[6];
    if(loadedVars & Var::F)  dst.f  = a[7];
    if(loadedVars & Var::G)  dst.g  = a[8];
}

void Storage::store(inout(Vars) src, uint storedPacks) {
    if(storedPacks & Var::A1) a[0] = src.a1;
    if(storedPacks & Var::A2) a[1] = src.a2;
    if(storedPacks & Var::B ) a[2] = src.b;
    if(storedPacks & Var::C1) a[3] = src.c1;
    if(storedPacks & Var::C2) a[4] = src.c2;
    if(storedPacks & Var::D)  a[5] = src.d;
    if(storedPacks & Var::E)  a[6] = src.e;
    if(storedPacks & Var::F)  a[7] = src.f;
    if(storedPacks & Var::G)  a[8] = src.g;
}