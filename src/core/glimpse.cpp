#include "glimpse.h"

using namespace glimpse;

const interval interval::empty = interval(+math::infinity, -math::infinity);
const interval interval::universe = interval(-math::infinity, +math::infinity);

const aabb aabb::empty = aabb(interval::empty, interval::empty, interval::empty);
const aabb aabb::universe = aabb(interval::universe, interval::universe, interval::universe);

// Initialize static members
thread_local std::mt19937 Random::generator;
thread_local bool Random::initialized = false;
uint32_t Random::global_seed = 0;
