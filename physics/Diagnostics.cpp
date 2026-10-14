//
// Created by tdragon on 9/19/26.
//

#include "Diagnostics.h"

#include <cmath>

SystemDiagnostics Diagnostics::calculate(
    const SolarSystem& solarSystem
)
{
    SystemDiagnostics result;

    const auto& bodies =
        solarSystem.getBodyPointers();

    double totalMass = 0.0;

    /*
     * Center of mass and momentum.
     */
    for (const Body* body : bodies)
    {
        if (!body->isActive())
            continue;

        const double mass =
            body->getMass();

        totalMass += mass;

        result.centerOfMass +=
            body->getPosition() *
            mass;

        result.centerOfMassVelocity +=
            body->getVelocity() *
            mass;

        result.totalMomentum +=
            body->getVelocity() *
            mass;

        result.kineticEnergy +=
            0.5 *
            mass *
            body->getVelocity().lengthSquared();
    }

    if (totalMass > 0.0)
    {
        result.centerOfMass /=
            totalMass;

        result.centerOfMassVelocity /=
            totalMass;
    }

    /*
     * Potential energy.
     *
     * Count every pair exactly once.
     */
    for (std::size_t i = 0;
         i < bodies.size();
         ++i)
    {
        const Body* first =
            bodies[i];

        if (!first->isActive())
            continue;

        for (std::size_t j = i + 1;
             j < bodies.size();
             ++j)
        {
            const Body* second =
                bodies[j];

            if (!second->isActive())
                continue;

            const Vec3 displacement =
                second->getPosition() -
                first->getPosition();

            const double distance =
                displacement.length();

            if (distance <= 0.0)
                continue;

            result.potentialEnergy -=
                first->getMass() *
                second->getMu() /
                distance;
        }
    }

    result.totalEnergy =
        result.kineticEnergy +
        result.potentialEnergy;

    /*
     * Total angular momentum about the
     * current origin.
     */
    for (const Body* body : bodies)
    {
        if (!body->isActive())
            continue;

        result.totalAngularMomentum +=
            body->getPosition().cross(
                body->getVelocity() *
                body->getMass()
            );
    }

    return result;
}