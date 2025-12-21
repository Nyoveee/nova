#pragma once

#include <glm/vec3.hpp>

struct Sphere {
    glm::vec3 position;
    float radius;
};

struct AABB {
    glm::vec3 center;
    glm::vec3 extent;
};

// Plane linear equation
// ax + by + cz + d = 0, where x, y and z are any points in R3 that satisfy the equation. 
struct Plane {
    glm::vec4 coefficients; // stores a, b, c and d.

    void normalize() {
        float mag;
        mag = sqrt(coefficients.x * coefficients.x + coefficients.y * coefficients.y + coefficients.z * coefficients.z);

        coefficients.x /= mag;
        coefficients.y /= mag;
        coefficients.z /= mag;
        coefficients.w /= mag;
    }

    float distanceToPoint(glm::vec3 position) const {
        return coefficients.x * position.x + coefficients.y * position.y + coefficients.z * position.z + coefficients.w;
    }

    bool isAABBInPlane(AABB const& aabb) const {
        // https://gdbooks.gitbooks.io/3dcollisions/content/Chapter2/static_aabb_plane.html
        // Compute the projection interval radius of b onto L(t) = b.c + t * p.n
        float r = 
            aabb.extent.x * std::abs(coefficients.x) +
            aabb.extent.y * std::abs(coefficients.y) +
            aabb.extent.z * std::abs(coefficients.z);

        return -r <= distanceToPoint(aabb.center);
    }
};

struct Frustum {
    Plane topPlane;
    Plane bottomPlane;

    Plane rightPlane;
    Plane leftPlane;

    Plane farPlane;
    Plane nearPlane;

    bool isSphereInFrustum(Sphere const& sphere) const {
        // Part of the sphere must be on the positive side of the plane if the distance between the sphere's center to the plane is > -radius.
        return 
                topPlane.distanceToPoint(sphere.position)    > -sphere.radius
            &&  bottomPlane.distanceToPoint(sphere.position) > -sphere.radius
            &&  rightPlane.distanceToPoint(sphere.position)  > -sphere.radius
            &&  leftPlane.distanceToPoint(sphere.position)   > -sphere.radius
            &&  farPlane.distanceToPoint(sphere.position)    > -sphere.radius
            &&  nearPlane.distanceToPoint(sphere.position)   > -sphere.radius
       ;
    }

    bool isAABBInFrustum(AABB const& aabb) const {
        return 
                topPlane.isAABBInPlane(aabb)
            &&  bottomPlane.isAABBInPlane(aabb)
            &&  rightPlane.isAABBInPlane(aabb)
            &&  leftPlane.isAABBInPlane(aabb)
            &&  farPlane.isAABBInPlane(aabb)
            &&  nearPlane.isAABBInPlane(aabb)
        ;
    }
};