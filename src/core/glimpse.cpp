#include "glimpse.h"

void donothing() {}

const interval interval::empty = interval(+infinity, -infinity);
const interval interval::universe = interval(-infinity, +infinity);

const aabb aabb::empty = aabb(interval::empty, interval::empty, interval::empty);
const aabb aabb::universe = aabb(interval::universe, interval::universe, interval::universe);

// Initialize static members
thread_local std::mt19937 Random::generator;
thread_local bool Random::initialized = false;
uint32_t Random::global_seed = 0;
