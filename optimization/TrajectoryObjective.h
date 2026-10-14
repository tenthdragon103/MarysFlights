//
// Created by tdragon on 9/21/26.
//

#ifndef MARYSFLIGHTS_TRAJECTORYOBJECTIVE_H
#define MARYSFLIGHTS_TRAJECTORYOBJECTIVE_H


#pragma once

#include "../physics/trajectory/Trajectory.h"
#include "../physics/trajectory/MissionRequest.h"
#include "../physics/trajectory/TrajectoryResult.h"

class TrajectoryObjective
{
public:
    double evaluate(
        const Trajectory& trajectory,
        const TrajectoryResult& result,
        const MissionRequest& mission
    ) const;

private:
    double calculateDeltaVCost(
        const Trajectory& trajectory,
        const MissionRequest& mission
    ) const;

    double calculateTimeCost(
        const Trajectory& trajectory,
        const MissionRequest& mission
    ) const;

    double calculateReversalPenalty(
        const Trajectory& trajectory
    ) const;

    double calculateBurnDurationPenalty(
        const Trajectory& trajectory
    ) const;
};


#endif //MARYSFLIGHTS_TRAJECTORYOBJECTIVE_H
