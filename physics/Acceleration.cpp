//
// Created by tdragon on 9/17/26.
//

#include "Acceleration.h"

#include "Gravity.h"
#include "raylib.h"

void Acceleration::calculate(
    SolarSystem& solarSystem
)
{
    const auto& bodies =
        solarSystem.getBodyPointers();

    for (Body* target : bodies)
    {
        if (!target->isActive())
            continue;

        Vec3 totalAcceleration{};

        for (Body* source : bodies)
        {
            if (source == target)
                continue;

            if (!source->isActive())
                continue;

            totalAcceleration +=
                Gravity::accelerationBetween(
                    *source,
                    *target
                );
        }

        target->setAcceleration(
            totalAcceleration
        );
    }
}
