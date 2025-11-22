#include "RandomRange.h"
#include <cstdlib>
float RandomRange::Float(float minInclusive, float maxInclusive)
{
    return static_cast<float>(rand()) / RAND_MAX * (maxInclusive - minInclusive) + minInclusive;
}

int RandomRange::Int(int minInclusive, int maxExclusive)
{
    return rand() % (maxExclusive - minInclusive) + minInclusive;
}
