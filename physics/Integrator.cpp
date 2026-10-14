//
// Created by tdragon on 9/17/26.
//

#include "Integrator.h"
#include "Acceleration.h"

void Integrator::step(
    SolarSystem& solarSystem,
    double deltaTime
)
{
    const auto& bodies =
        solarSystem.getBodyPointers();

    const double halfDeltaTime =
        deltaTime * 0.5;

    /*
     * Half velocity step.
     */
    for (Body* body : bodies)
    {
        if (!body->isActive())
            continue;

        body->setVelocity(
            body->getVelocity() +
            body->getAcceleration() *
            halfDeltaTime
        );
    }

    /*
     * Position step.
     */
    for (Body* body : bodies)
    {
        if (!body->isActive())
            continue;

        body->setPosition(
            body->getPosition() +
            body->getVelocity() *
            deltaTime
        );
    }

    /*
     * Calculate acceleration at the
     * new positions.
     */
    Acceleration::calculate(
        solarSystem
    );

    /*
     * Finish velocity step.
     */
    for (Body* body : bodies)
    {
        if (!body->isActive())
            continue;

        body->setVelocity(
            body->getVelocity() +
            body->getAcceleration() *
            halfDeltaTime
        );
    }

    // TODO: integrate the agentic integrator and call down the next agent decision
}