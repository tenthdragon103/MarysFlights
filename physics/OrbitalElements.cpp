//
// Created by tdragon on 9/19/26.
//

#include "OrbitalElements.h"

#include <cmath>

namespace
{
    constexpr double PI =
        3.1415926535897932384626433832795;

    constexpr double TWO_PI =
        2.0 * PI;

    double normalizeAngle(
        double angle
    )
    {
        angle =
            std::fmod(
                angle,
                TWO_PI
            );

        if (angle < 0.0)
            angle += TWO_PI;

        return angle;
    }
}

double OrbitalElementsSolver::solveEccentricAnomaly(
    double meanAnomaly,
    double eccentricity
)
{
    double E = meanAnomaly;

    /*
     * Newton-Raphson solution of:
     *
     * E - e sin(E) = M
     */
    for (int i = 0; i < 20; ++i)
    {
        const double f =
            E -
            eccentricity * std::sin(E) -
            meanAnomaly;

        const double derivative =
            1.0 -
            eccentricity * std::cos(E);

        const double correction =
            f / derivative;

        E -= correction;

        if (std::abs(correction) < 1e-14)
            break;
    }

    return E;
}

OrbitalState OrbitalElementsSolver::toCartesian(
    const OrbitalElements& elements,
    double gravitationalParameter
)
{
    const double a =
        elements.semiMajorAxis;

    const double e =
        elements.eccentricity;

    const double i =
        elements.inclination;

    const double omega =
        elements.argumentOfPeriapsis;

    const double Omega =
        elements.longitudeOfAscendingNode;

    const double M =
        normalizeAngle(
            elements.meanAnomaly
        );

    /*
     * Solve Kepler's equation.
     */
    const double E =
        solveEccentricAnomaly(
            M,
            e
        );

    /*
     * Position in orbital plane.
     */
    const double cosE =
        std::cos(E);

    const double sinE =
        std::sin(E);

    const double radius =
        a *
        (1.0 - e * cosE);

    const double sqrtOneMinusESquared =
        std::sqrt(
            1.0 - e * e
        );

    const double cosNu =
        (cosE - e) /
        (1.0 - e * cosE);

    const double sinNu =
        (
            sqrtOneMinusESquared *
            sinE
        ) /
        (1.0 - e * cosE);

    /*
     * Perifocal position.
     */
    const Vec3 rPerifocal = {
        radius * cosNu,
        radius * sinNu,
        0.0
    };

    /*
     * Perifocal velocity.
     */
    const double velocityScale =
        std::sqrt(
            gravitationalParameter * a
        ) /
        radius;

    const Vec3 vPerifocal = {
        -velocityScale * sinE,
        velocityScale *
            sqrtOneMinusESquared *
            cosE,
        0.0
    };

    /*
     * Rotation:
     *
     * R3(Omega)
     * R1(i)
     * R3(omega)
     */

    const double cosOmega =
        std::cos(Omega);

    const double sinOmega =
        std::sin(Omega);

    const double cosInclination =
        std::cos(i);

    const double sinInclination =
        std::sin(i);

    const double cosOmegaArg =
        std::cos(omega);

    const double sinOmegaArg =
        std::sin(omega);

    const Vec3 basisP = {
        cosOmega * cosOmegaArg -
            sinOmega * sinOmegaArg *
            cosInclination,

        sinOmega * cosOmegaArg +
            cosOmega * sinOmegaArg *
            cosInclination,

        sinOmegaArg * sinInclination
    };

    const Vec3 basisQ = {
        -cosOmega * sinOmegaArg -
            sinOmega * cosOmegaArg *
            cosInclination,

        -sinOmega * sinOmegaArg +
            cosOmega * cosOmegaArg *
            cosInclination,

        cosOmegaArg * sinInclination
    };

    OrbitalState state;

    state.position =
        basisP * rPerifocal.x +
        basisQ * rPerifocal.y;

    state.velocity =
        basisP * vPerifocal.x +
        basisQ * vPerifocal.y;

    return state;
}