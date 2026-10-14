//
// Created by tdragon on 9/20/26.
//

#ifndef MARYSFLIGHTS_BURN_H
#define MARYSFLIGHTS_BURN_H


#pragma once

#include "../../math/Vec3.h"
#include "../../bodies/Body.h"

struct Burn
{
    double startTime = 0.0;
    double duration = 0.0;

    Vec3 deltaVelocity;

    Body* referenceBody = nullptr;

    [[nodiscard]] double getDeltaV() const
    {
        return deltaVelocity.length();
    };

    [[nodiscard]] double getEndTime() const
    {
        return startTime + duration;
    };
};


#endif //MARYSFLIGHTS_BURN_H
