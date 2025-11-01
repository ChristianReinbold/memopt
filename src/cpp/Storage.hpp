#pragma once

#include <Kernel.hpp>
#include <StorageBase.hpp>

#include <iostream>
#include <sstream>

std::ostream& operator <<(std::ostream& stream, const Storage::Var&& v) {
    bool delimRequired = false;
    auto delim = [&]() {
        if (delimRequired) stream << ",";
        delimRequired = true;
    };
    if(v & Storage::Var::A1) { delim(); stream << "A1"; }
    if(v & Storage::Var::A2) { delim(); stream << "A2"; }
    if(v & Storage::Var::B)  { delim(); stream << "B";  }
    if(v & Storage::Var::C1) { delim(); stream << "C1"; }
    if(v & Storage::Var::C2) { delim(); stream << "C2"; }
    if(v & Storage::Var::D)  { delim(); stream << "D";  }
    if(v & Storage::Var::E)  { delim(); stream << "E";  }
    if(v & Storage::Var::F)  { delim(); stream << "F";  }
    if(v & Storage::Var::G)  { delim(); stream << "G";  }
    return stream;
}

std::ostream& operator <<(std::ostream& stream, const Storage::Pack&& v) {
    bool delimRequired = false;
    auto delim = [&]() {
        if (delimRequired) stream << ",";
        delimRequired = true;
    };
    if(v & Storage::Pack::A) { delim(); stream << "A"; }
    if(v & Storage::Pack::B) { delim(); stream << "B"; }
    if(v & Storage::Pack::C) { delim(); stream << "C"; }
    if(v & Storage::Pack::D) { delim(); stream << "D"; }
    if(v & Storage::Pack::E) { delim(); stream << "E"; }
    if(v & Storage::Pack::F) { delim(); stream << "F"; }
    if(v & Storage::Pack::G) { delim(); stream << "G"; }
    return stream;
}

void Storage::load(inout(Vars) dst, uint loadedPacks, uint loadedVars) {
    if (loadedPacks == 0) {
        return;
    }
    std::cout << "Load packs [" << Pack(loadedPacks)
              << "] and read vars [" << Var(loadedVars) << "]\n";
}

void Storage::store(inout(Vars) src, uint storedPacks) {
    if (storedPacks == 0) {
        return;
    }
    std::cout << "Store packs [" << Pack(storedPacks) << "]\n";
}
