//
// Created by tdragon on 9/21/26.
//

#ifndef MARYSFLIGHTS_TRAJECTORYRESULT_H
#define MARYSFLIGHTS_TRAJECTORYRESULT_H


#pragma once

#include "../../math/Vec3.h"

#include <string>

struct TrajectoryResult
{
    /*
     * Overall outcome.
     */
    bool completed = false;
    bool reachedTarget = false;
    bool captured = false;

    bool collided = false;
    bool escaped = false;

    /*
     * Final spacecraft state.
     */
    Vec3 finalPosition;
    Vec3 finalVelocity;

    /*
     * Final state relative to target.
     */
    Vec3 targetRelativePosition;
    Vec3 targetRelativeVelocity;

    /*
     * Closest approach to target during
     * the entire trajectory.
     */
    double closestApproachDistance = 0.0;

    /*
     * Distance from target at the end.
     */
    double finalDistance = 0.0;

    /*
     * Relative velocity at the end.
     */
    double finalRelativeSpeed = 0.0;

    /*
     * Actual simulated flight time.
     */
    double flightTime = 0.0;

    /*
     * Total delta-v used.
     */
    double totalDeltaV = 0.0;

    /*
     * Remaining delta-v budget.
     */
    double remainingDeltaV = 0.0;

    /*
     * Delta-v categories.
     */
    double departureDeltaV = 0.0;
    double correctionDeltaV = 0.0;
    double captureDeltaV = 0.0;

    /*
     * Maneuver statistics.
     */
    int burnsExecuted = 0;
    int gravityAssists = 0;

    /*
     * Target state.
     */
    double targetOrbitalVelocity = 0.0;
    double targetEscapeVelocity = 0.0;

    /*
     * Validity / failure information.
     */
    bool valid = true;
    std::string failureReason;

    bool hasFailed() const;
    bool isSuccessful() const;
};


#endif //MARYSFLIGHTS_TRAJECTORYRESULT_H
