//
// Created by tdragon on 9/17/26.
//

#include "ObjectPicker.h"

#include <cmath>
#include <limits>

Ray ObjectPicker::getMouseRay(
    const Camera3D& camera
)
{
    return GetMouseRay(
        GetMousePosition(),
        camera
    );
}

bool ObjectPicker::intersectsSphere(
    const Ray& ray,
    const Vector3& center,
    float radius,
    float& distance
)
{
    const Vector3 oc = {
        ray.position.x - center.x,
        ray.position.y - center.y,
        ray.position.z - center.z
    };

    const float b =
        oc.x * ray.direction.x +
        oc.y * ray.direction.y +
        oc.z * ray.direction.z;

    const float c =
        oc.x * oc.x +
        oc.y * oc.y +
        oc.z * oc.z -
        radius * radius;

    const float discriminant =
        b * b - c;

    if (discriminant < 0.0f)
        return false;

    const float sqrtDiscriminant =
        std::sqrt(discriminant);

    const float t1 =
        -b - sqrtDiscriminant;

    const float t2 =
        -b + sqrtDiscriminant;

    if (t1 >= 0.0f)
    {
        distance = t1;
        return true;
    }

    if (t2 >= 0.0f)
    {
        distance = t2;
        return true;
    }

    return false;
}

const Body* ObjectPicker::pickBody(
    const Camera3D& camera,
    const SolarSystem& solarSystem,
    double solarScale,
    double minimumRadius
)
{
    const Ray ray =
        getMouseRay(camera);

    const Body* closestBody = nullptr;

    float closestDistance =
        std::numeric_limits<float>::max();

    for (const Body* body :
         solarSystem.getBodyPointers())
    {
        if (!body->isActive())
            continue;

        const Vector3 center = {
            static_cast<float>(
                body->getPosition().x *
                solarScale
            ),

            static_cast<float>(
                body->getPosition().z *
                solarScale
            ),

            static_cast<float>(
                body->getPosition().y *
                solarScale
            )
        };

        const float physicalRadius =
            static_cast<float>(
                body->getRadius() *
                solarScale
            );

        const float radius =
            std::max(
                physicalRadius,
                static_cast<float>(
                    minimumRadius
                )
            );

        float distance = 0.0f;

        if (!intersectsSphere(
                ray,
                center,
                radius,
                distance))
        {
            continue;
        }

        if (distance < closestDistance)
        {
            closestDistance = distance;
            closestBody = body;
        }
    }

    return closestBody;
}