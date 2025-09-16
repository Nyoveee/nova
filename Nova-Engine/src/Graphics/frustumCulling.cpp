#include "frustumCulling.h"

constexpr float cEpsilon = 1e-5f;

aabb::aabb(glm::vec3 const& min, glm::vec3 const& max) : min(min), max(max) {}

plane::plane(glm::vec3 const& point, glm::vec3 const& normal) : normal(glm::normalize(normal)), distance(glm::dot(glm::normalize(normal), point)) {}

plane::plane(glm::vec3 const& normal, float dist) : normal(glm::normalize(normal)), distance(dist) {}

cullResult plane::checkFrustumCulling(const sphere& sphere) const {
    float _distance = glm::dot(normal, sphere.center) - distance;
    if (_distance > sphere.radius)
    {
        return isOutside;
    }
    if (_distance < -sphere.radius)
    {
        return isInside;
    }

    return isIntersecting;
}

sphere::sphere(glm::vec3 const& center, float radius) : center(center), radius(radius) {}



void frustum::createFrustumFromCamera(const Camera& cam, float aspect, float fovY, float zNear, float zFar) {
    const float halfVSide = zFar * tanf(fovY * .5f);
    const float halfHSide = halfVSide * aspect;
    const glm::vec3 frontMultFar = zFar * cam.getFront();

    nearPlane = { cam.getPos() + zNear * cam.getFront(), cam.getFront() };
    farPlane = { cam.getPos() + frontMultFar, -cam.getFront() };
    right = { cam.getPos(),
                  glm::cross(frontMultFar - cam.getRight() * halfHSide, cam.Up) };
    left = { cam.getPos(),
                  glm::cross(cam.Up, frontMultFar + cam.getRight() * halfHSide) };
    top = { cam.getPos(),
                  glm::cross(cam.getRight(), frontMultFar - cam.Up * halfVSide) };
    btm = { cam.getPos(),
                  glm::cross(frontMultFar + cam.Up * halfVSide, cam.getRight()) };

    normals = { left.normal, right.normal, btm.normal, top.normal, nearPlane.normal, farPlane.normal };
    dists = { left.distance, right.distance, btm.distance, top.distance, nearPlane.distance, farPlane.distance };

    //// debug: print plane normals and distances
    //for (int i = 0; i < 6; ++i) {
    //    auto N = normals[i];
    //    float D = dists[i];
    //    printf("plane %d normal=(%f %f %f) D=%f\n", i, N.x, N.y, N.z, D);
    //}
    //// test point: camera position should be inside all side-planes (distance >= 0)
    //glm::vec3 camPos = cam.getPos();
    //for (int i = 0; i < 6; ++i) {
    //    float dist = glm::dot(normals[i], camPos) - dists[i];
    //    printf("plane %d at camPos dist = %f\n", i, dist);
    //}
}

//std::array<glm::vec3, 8> frustum::getFrustumCorners(const Camera& cam)
//{
//    float nearZ = cam.getNearPlane();
//    float farZ = cam.getFarPlane();
//    float fov = glm::radians(cam.getFov());
//    float aspect = cam.getAspectRatio();
//
//    glm::vec3 pos = cam.getPos();
//    glm::vec3 forward = cam.getFront();
//    glm::vec3 right = cam.getRight();
//    glm::vec3 up = cam.Up;
//
//    float nearHeight = 2.0f * tan(fov * 0.5f) * nearZ;
//    float nearWidth = nearHeight * aspect;
//    float farHeight = 2.0f * tan(fov * 0.5f) * farZ;
//    float farWidth = farHeight * aspect;
//
//    glm::vec3 nc = pos + forward * nearZ; // near center
//    glm::vec3 fc = pos + forward * farZ;  // far center
//
//    std::array<glm::vec3, 8> corners;
//    // Near plane
//    corners[0] = nc + (up * (nearHeight / 2.0f)) - (right * (nearWidth / 2.0f)); // ntl
//    corners[1] = nc + (up * (nearHeight / 2.0f)) + (right * (nearWidth / 2.0f)); // ntr
//    corners[2] = nc - (up * (nearHeight / 2.0f)) - (right * (nearWidth / 2.0f)); // nbl
//    corners[3] = nc - (up * (nearHeight / 2.0f)) + (right * (nearWidth / 2.0f)); // nbr
//
//    // Far plane
//    corners[4] = fc + (up * (farHeight / 2.0f)) - (right * (farWidth / 2.0f));   // ftl
//    corners[5] = fc + (up * (farHeight / 2.0f)) + (right * (farWidth / 2.0f));   // ftr
//    corners[6] = fc - (up * (farHeight / 2.0f)) - (right * (farWidth / 2.0f));   // fbl
//    corners[7] = fc - (up * (farHeight / 2.0f)) + (right * (farWidth / 2.0f));   // fbr
//
//    return corners;
//}


cullResult frustum::checkFrustumCulling(const sphere& sphere) const
{
    bool intersects = false;

    for (int i = 0; i < 6; i++) {
        float d = glm::dot(normals[i], sphere.center) - dists[i];

        if (d < -sphere.radius) {
            return cullResult::isOutside; // completely outside
        }
        if (d < sphere.radius) {
            intersects = true; // intersects this plane
        }
    }

    return intersects ? cullResult::isIntersecting
        : cullResult::isInside;
}

cullResult frustum::checkFrustumCulling(const aabb& aabb) const
{
    int outside_count = 0;

    for (size_t i = 0; i < normals.size(); ++i) {
        const glm::vec3& N = normals[i];
        float D = dists[i];

        // Find the positive vertex (farthest along normal direction)
        glm::vec3 p_vertex;
        p_vertex.x = (N.x >= 0 ? aabb.max.x : aabb.min.x);
        p_vertex.y = (N.y >= 0 ? aabb.max.y : aabb.min.y);
        p_vertex.z = (N.z >= 0 ? aabb.max.z : aabb.min.z);

        // If positive vertex is outside any plane, entire box is outside
        float p_dist = glm::dot(N, p_vertex) - D;
        if (p_dist < -cEpsilon) {
            return isOutside;
        }

        // Count how many planes the negative vertex is outside
        glm::vec3 n_vertex;
        n_vertex.x = (N.x >= 0 ? aabb.min.x : aabb.max.x);
        n_vertex.y = (N.y >= 0 ? aabb.min.y : aabb.max.y);
        n_vertex.z = (N.z >= 0 ? aabb.min.z : aabb.max.z);

        float n_dist = glm::dot(N, n_vertex) - D;
        if (n_dist < -cEpsilon) {
            outside_count++;
        }
    }

    // Only return intersecting if multiple planes are intersected
    // This reduces the number of intersecting cases
    return (outside_count >= 2) ? isIntersecting : isInside;
}