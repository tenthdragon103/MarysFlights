//
// Created by tdragon on 9/19/26.
//

#ifndef MARYSFLIGHTS_ORBITALELEMENTS_H
#define MARYSFLIGHTS_ORBITALELEMENTS_H

#pragma once

#include "../math/Vec3.h"

struct OrbitalElements
{
    double semiMajorAxis = 0.0;
    double eccentricity = 0.0;

    double inclination = 0.0;
    double longitudeOfAscendingNode = 0.0;
    double argumentOfPeriapsis = 0.0;

    double meanAnomaly = 0.0;

    double epochJulianDate = 0.0;
};

struct OrbitalState
{
    Vec3 position;
    Vec3 velocity;
};

class OrbitalElementsSolver
{
public:
    static OrbitalState toCartesian(
        const OrbitalElements& elements,
        double gravitationalParameter
    );

private:
    static double solveEccentricAnomaly(
        double meanAnomaly,
        double eccentricity
    );
};


#endif //MARYSFLIGHTS_ORBITALELEMENTS_H
