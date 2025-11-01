#pragma once

#include <iostream>

#define __CPP__

typedef unsigned int uint;

#define inout(T) T&
#define INIT(T) T{}
#define PRIVATE private:
#define PUBLIC public:
#define STATIC_ASSERT(T) static_assert(T)
#define PRINT(X) std::cout << X