//
// Created by tdragon on 9/19/26.
//

#ifndef MARYSFLIGHTS_DIAGNOSTICS_H
#define MARYSFLIGHTS_DIAGNOSTICS_H


#pragma once

#include "../bodies/SolarSystem.h"

struct SystemDiagnostics
{
    Vec3 centerOfMass;
    Vec3 centerOfMassVelocity;

    Vec3 totalMomentum;
    Vec3 totalAngularMomentum;

    double kineticEnergy = 0.0;
    double potentialEnergy = 0.0;
    double totalEnergy = 0.0;
};

class Diagnostics
{
public:
    static SystemDiagnostics calculate(
        const SolarSystem& solarSystem
    );
};


#endif //MARYSFLIGHTS_DIAGNOSTICS_H
