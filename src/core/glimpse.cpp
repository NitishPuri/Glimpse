#include "glimpse.h"

void donothing() {}

const interval interval::empty = interval(+infinity, -infinity);
const interval interval::universe = interval(-infinity, +infinity);

const aabb aabb::empty =
    aabb(interval::empty, interval::empty, interval::empty);
const aabb aabb::universe =
    aabb(interval::universe, interval::universe, interval::universe);