#pragma once

#include <cstdlib>

#define S2D_ASSERT(expr, msg) if (!(expr)) { printf("Assertion (%s) failed on line %i: %s", #expr, __LINE__, msg); std::terminate(); }