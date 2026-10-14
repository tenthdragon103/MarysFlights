//
// Created by tdragon on 9/17/26.
//

#ifndef MARYSFLIGHTS_INTEGRATOR_H
#define MARYSFLIGHTS_INTEGRATOR_H

#pragma once

#include "../bodies/SolarSystem.h"

class Integrator
{
public:
    static void step(
        SolarSystem& solarSystem,
        double deltaTime
    );
};


#endif //MARYSFLIGHTS_INTEGRATOR_H
