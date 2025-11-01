// Provides useful template constructs for debugging and asserting template code.

#pragma once

#include <Kernel.hpp>

// Create a unique variable name, see
// https://stackoverflow.com/questions/1082192/how-to-generate-random-variable-names-in-c-using-macros
#define CONCAT(a, b) CONCAT_INNER(a, b)
#define CONCAT_INNER(a, b) a ## b
#define TEMPLATE_ASSERT(Cond, A, B) Cond<A, B> CONCAT(__TemplateAssert, __COUNTER__)

template<uint A, uint B> struct AssertDisjoint {
  STATIC_ASSERT((A & B) == 0);
  static const bool value = ((A & B) == 0);
};

template<uint A, uint B> struct AssertImplies {
  STATIC_ASSERT((A & ~B) == 0);
  static const bool value = ((A & ~B) == 0);
};

template<uint A, uint B> struct AssertEqual {
  STATIC_ASSERT(A == B);
  static const bool value = (A == B);
};