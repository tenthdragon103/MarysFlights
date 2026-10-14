//
// Created by tdragon on 9/17/26.
//

#include "Gravity.h"

Vec3 Gravity::accelerationBetween(
    const Body& source,
    const Body& target
)
{
    const Vec3 displacement =
        source.getPosition() -
        target.getPosition();

    const double distanceSquared =
        displacement.lengthSquared();

    if (distanceSquared <= 0.0)
        return {};

    const double inverseDistance =
        1.0 / std::sqrt(distanceSquared);

    const double inverseDistanceCubed =
        inverseDistance *
        inverseDistance *
        inverseDistance;

    return displacement *
           (source.getMu() *
            inverseDistanceCubed);
}