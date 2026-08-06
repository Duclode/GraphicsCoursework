#ifndef COLLISION_H
#define COLLISION_H

#include <glm/glm.hpp>
#include "mesh.h"

struct Sphere
{
    glm::vec3 center;
    float radius;
};

bool SphereIntersectsAABB(const Sphere& sphere, const AABB& box);

#endif
