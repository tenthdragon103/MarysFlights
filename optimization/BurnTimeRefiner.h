//
// Created by tdragon on 9/21/26.
//

#ifndef MARYSFLIGHTS_BURNTIMEREFINER_H
#define MARYSFLIGHTS_BURNTIMEREFINER_H


#pragma once

#include "../physics/trajectory/Trajectory.h"
#include "../physics/trajectory/MissionRequest.h"
#include "../physics/trajectory/TrajectoryResult.h"

#include <functional>

class BurnTimeRefiner
{
public:
    using EvaluateFunction =
        std::function<double(
            const Trajectory&
        )>;

    Trajectory refine(
        const Trajectory& initial,
        const MissionRequest& mission,
        const EvaluateFunction& evaluate
    ) const;

private:
    bool tryImproveBurn(
        Trajectory& trajectory,
        std::size_t burnIndex,
        double step,
        double minimumGap,
        const EvaluateFunction& evaluate
    ) const;
};


#endif //MARYSFLIGHTS_BURNTIMEREFINER_H
