//
// Created by tdragon on 9/17/26.
//

#ifndef MARYSFLIGHTS_ACCELERATION_H
#define MARYSFLIGHTS_ACCELERATION_H

#pragma once

#include "../bodies/SolarSystem.h"

class Acceleration
{
public:
    static void calculate(
        SolarSystem& solarSystem
    );
};

#endif //MARYSFLIGHTS_ACCELERATION_H
