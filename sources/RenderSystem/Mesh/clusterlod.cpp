// Non-module TU: clusterlod.h's implementation needs the textual meshoptimizer
// header; module code sees only its declarations (MeshletGeneration.cpp).
#include <cassert>
#include <meshoptimizer.h>

#define CLUSTERLOD_IMPLEMENTATION
#include "clusterlod.h"
