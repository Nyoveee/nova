#include "Interpolation.h"
#include <cmath>
float Interpolation::Interpolation(float start, float end, float t, float degree)
{
    assert(degree > 0 && "Interpolation degree must be more than zero");
    return start + (end - start) * static_cast<float>(std::pow(t, 1.f / degree));
}
glm::vec4 Interpolation::Interpolation(glm::vec4 start, glm::vec4 end, float t, float degree) {
    float x = Interpolation(start.x, end.x, t, degree);
    float y = Interpolation(start.y, end.y, t, degree);
    float z = Interpolation(start.z, end.z, t, degree);
    float w = Interpolation(start.w, end.w, t, degree);
    return glm::vec4{ x,y,z,w };
}
glm::vec3 Interpolation::Interpolation(glm::vec3 start, glm::vec3 end, float t, float degree)
{
    float x = Interpolation(start.x, end.x, t, degree);
    float y = Interpolation(start.y, end.y, t, degree);
    float z = Interpolation(start.z, end.z, t, degree);
    return glm::vec3{ x,y,z };
}

glm::vec2 Interpolation::Interpolation(glm::vec2 start, glm::vec2 end, float t, float degree)
{
    float x = Interpolation(start.x, end.x, t, degree);
    float y = Interpolation(start.x, end.x, t, degree);
    return glm::vec2{ x,y };
}

ColorA Interpolation::Interpolation(ColorA start, ColorA end, float t, float degree)
{
    float r = Interpolation(start.r(), end.r(), t, degree);
    float g = Interpolation(start.g(), end.g(), t, degree);
    float b = Interpolation(start.b(), end.b(), t, degree);
    float a = Interpolation(start.a(), end.a(), t, degree);
    return ColorA{ r,g,b,a };
}

Color Interpolation::Interpolation(Color start, Color end, float t, float degree)
{
    float r = Interpolation(start.r(), end.r(), t, degree);
    float g = Interpolation(start.g(), end.g(), t, degree);
    float b = Interpolation(start.b(), end.b(), t, degree);
    return Color{ r,g,b };
}
