Memory Access Optimization
==========================

This repository provides a framework to autogenerate optimized read and write operations of memory outside the control of the compiler. It is designed and deployed in the context of GPU programming, more specifically HLSL shader optimization, to improve performance of complex, memory-bottlenecked shaders by limiting (global) memory-register data transfers. Still, the framework may be used in various other contexts thanks to its general-purpose design and C++ language support.

The current implementation is **proof of concept** work, and serves as a playground for grabbing some experience in compiler optimization and template metaprogramming. A heavily simplified version of this framework made it into a small production codebase to automatically elide superfluous reads and writes of global memory. It improved performance by double digit percentages, which was quite nice. Still, I **highly discourage** going this route for any project anyone else except you has to work with.


I might pick up this project again in a context where it is better suited, probably as an optimization pass in [SPIRV-Tools](https://github.com/KhronosGroup/SPIRV-Tools). If you still are curious what I tried to achieve, here are my original design goals:

Performance
-----------
* **P1 - Minimal Data Transfer**: Each buffer is accessed by at most one load and one store operation, even in the presence of branch divergence.
* **P2 - Code Sinking**: Load-store operations are moved into less frequently-visited conditional branches whenever this is possible without violating correctness or the uniqueness guarantee (P1).
* **P3 - Minimal Live State**: Load-store operations are delayed or pulled forward such that register usage is minimized without regressing with respect to P1 or P2.
* **P4 - No Overhead**: No run-time overhead is incurred by the framework. That is, scheduling load-store operations with the framework is equally fast compared to scheduling and hardcoding the operations manually.

Usability
---------
* **U1 - Locality**: To enable load-store scheduling, users of the framework explicitly have to provide information about control & data flow of their system to the framework. This information can be specified close to the source locations where the actual flow is implemented. Thus, the framework does not necessitate any non-local code modifications if the code-base changes in the future.
* **U2 - Decoupling**: Framework concepts are is decoupled from the actual shader logic. That is, only minimal changes to existing classes and functions are required when integrating the framework into existing code-bases.
* **U3 - Linar Compile-Time Complexity**: The core of the framework is implemented in a template engine that executes at compile-time. The size of template-generated code scales linearly with the size of the original code-base after inlining some specific functions. Compile-Times thus do not get out of hand.
* **U4 - Robustness**: The chance of unintentional misuse of the framework, e.g. by providing wrong flow information or missing out on invoking framework functions, is minimized. Whenever possible without compromising U2, faulty code will be detected at compile-time by non-matching types or static assertions. Naming schemes used in the framework are designed in such a way that misuse results in obvious code-smell. Lastly, the framework provide simple means to validate its functionality and correct usage at unit-test time, provided that coverage tests are available. 
* **U5 - Debugability**: The internal state of the framework can be printed in both human- and machine-readable formats at various levels of verbosity. In particular, users of the framework are able to quickly asses where load/store operations are scheduled to. This feature facilitates custom code analysis and debugging sessions.

How to build
------------

If you are familiar with CMake it should be very straightforward to build this project with Clang or MSVC. If you want to see the framework operating in the context of HLSL, you can also build a [patched version](0001-make-constexpr-and-nested-aliases-available.patch) of DXC on your own, with some quick hacks to enable constexpr.

License
-------

This project is licensed under the [MIT License](LICENSE).