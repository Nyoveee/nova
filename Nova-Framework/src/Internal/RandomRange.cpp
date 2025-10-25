#include "RandomRange.h"
#include <cstdlib>
float RandomRange::Float(float min, float max)
{
    return static_cast<float>(rand()) / RAND_MAX * (max - min) + min;
}

int RandomRange::Int(int min, int max)
{
    return rand() % (max - min + 1) + min;
}
