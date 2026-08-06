#include <collision.h>

bool SphereIntersectsAABB(const Sphere& sphere, const AABB& box)
{
    // find the closest point on the box to the sphere's centre
    glm::vec3 closestPoint;

    if (sphere.center.x < box.min.x)
        closestPoint.x = box.min.x;
    else if (sphere.center.x > box.max.x)
        closestPoint.x = box.max.x;
    else
        closestPoint.x = sphere.center.x;

    if (sphere.center.y < box.min.y)
        closestPoint.y = box.min.y;
    else if (sphere.center.y > box.max.y)
        closestPoint.y = box.max.y;
    else
        closestPoint.y = sphere.center.y;

    if (sphere.center.z < box.min.z)
        closestPoint.z = box.min.z;
    else if (sphere.center.z > box.max.z)
        closestPoint.z = box.max.z;
    else
        closestPoint.z = sphere.center.z;


    // calculate distance between the sphere's centre and closest point
    glm::vec3 distance = sphere.center - closestPoint;

    float distanceSquared =
        distance.x * distance.x +
        distance.y * distance.y +
        distance.z * distance.z;


    // If the distance is smaller than the radius, they collide
    return distanceSquared <= sphere.radius * sphere.radius;
}
