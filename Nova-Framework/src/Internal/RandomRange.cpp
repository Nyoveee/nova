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

glm::vec2 RandomRange::Vec2(glm::vec2 min, glm::vec2 max)
{
    return glm::vec2{ Float(min.x,max.x), Float(min.y,max.y) };
}

 glm::vec3 RandomRange::Vec3(glm::vec3 min, glm::vec3 max)
{
    return glm::vec3{ Float(min.x,max.x),Float(min.y,max.y), Float(min.z,max.z)};
}

glm::vec4 RandomRange::Vec4(glm::vec4 min, glm::vec4 max)
{
    return glm::vec4{ Float(min.x,max.x),Float(min.y,max.y), Float(min.z,max.z), Float(min.w,max.w) };
}