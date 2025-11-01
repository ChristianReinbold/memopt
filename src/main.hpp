#include <Kernel.hpp>
#include <Storage.hpp>
#include <StorageAccess/StorageAccess.hpp>
#include <StorageAccess/_Print.hpp>

#ifndef __HLSL__
#include <iostream>
#endif

// Test Switch-Case

// Variables: A1, A2, B, C1, C2, D, E, F, G

// Before merge: Force available & dirty of G
// Case 1: r=A1 w=B, G
// Case 2: r=A2 w=B, D
// Case 3: r=B  w=B, E, F
// Case 4: r=C1 w=B, F
// After merge: Force live B, C1, D

// --- Early Load ---

// "B" is all modified case => do not load early, but load in Case 3
// "C1" loaded once, but also required later => load early
// "A1/A2" => load early
// "D" desync with some write => load early
// "E" desyncs, but not used later => do not load
// "F" desyncs in two but not all branches => load early & delay store

// --- Delay Store ---

// "G" dirty before cases already => delay G
// "D" not dirty before, only modified in one branch => do not delay
// "E" not dirty before, only modified in one branch, but also changes later => do not delay explicitdly
// "F" not dirty before, modified in two branches => delayed

using Case1 = BasicBlock<Var::A1, 0u, Var::B | Var::G>;
using Case2 = BasicBlock<Var::A2, 0u, Var::B | Var::D>;
using Case3 = BasicBlock<Var::B, 0u, Var::B | Var::E | Var::F>;
using Case4 = BasicBlock<Var::C1, 0u, Var::B | Var::F>;
using AfterSwitch = BasicBlock<Var::B | Var::C1 | Var::D, 0u, 0u>;
using BeforeSwitch = BasicBlock<0u, 0u, Var::G>;

using CS = Sequence3<BeforeSwitch, Switch4<Case1, Case2, Case3, Case4>, AfterSwitch>;

uint entryPoint()
{
    Storage storage = INIT(Storage);
    using ACS = CS::Annotated<Storage>;
    ACS acs = CS::bind(storage);
    

#ifndef __HLSL__
    acs.stream(std::cout, PrintInfo(PrintFlags::User));   
    //acs.stream(std::cout, PrintInfo(PrintFlags::Verbose));
    PRINT("\n\n----------------------\n\n");
#endif

    Vars v = INIT(Vars);

    acs.b1.begin(v);
    PRINT("Doing stuff before Switch\n");
    acs.b1.end(v);

    acs.b2.begin(v);
    {
        acs.b2.c1.begin(v);
        PRINT("Doing stuff in Case 1\n");
        acs.b2.c1.end(v);

        acs.b2.c2.begin(v);
        PRINT("Doing stuff in Case 2\n");
        acs.b2.c2.end(v);

        acs.b2.c3.begin(v);
        PRINT("Doing stuff in Case 3\n");
        acs.b2.c3.end(v);

        acs.b2.c4.begin(v);
        PRINT("Doing stuff in Case 4\n");
        acs.b2.c4.end(v);
    }
    acs.b2.end(v);
    
    acs.b3.begin(v);
    PRINT("Doing stuff after Switch\n");
    acs.b3.end(v);

    return 0;
}
