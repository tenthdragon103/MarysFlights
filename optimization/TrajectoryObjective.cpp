//
// Created by tdragon on 9/21/26.
//

#include "TrajectoryObjective.h"

#include <algorithm>
#include <cmath>
#include <limits>

double TrajectoryObjective::evaluate(
    const Trajectory& trajectory,
    const TrajectoryResult& result,
    const MissionRequest& mission
) const
{
    if (!result.reachedTarget)
        return 1e9 + result.finalDistance;

    if (mission.requireCapture &&
        !result.captured)
    {
        return 1e8 + result.finalDistance;
    }

    const double deltaVCost =
        calculateDeltaVCost(
            trajectory,
            mission
        );

    const double timeCost =
        calculateTimeCost(
            trajectory,
            mission
        );

    const double reversalPenalty =
        calculateReversalPenalty(
            trajectory
        );

    const double durationPenalty =
        calculateBurnDurationPenalty(
            trajectory
        );

    return
        deltaVCost +
        timeCost +
        reversalPenalty +
        durationPenalty;
}

double TrajectoryObjective::calculateDeltaVCost(
    const Trajectory& trajectory,
    const MissionRequest& mission
) const
{
    if (mission.deltaVBudget <= 0.0)
        return 1e9;

    const double fraction =
        trajectory.getTotalDeltaV() /
        mission.deltaVBudget;

    if (fraction > 1.0)
        return 1e7 * fraction;

    return std::pow(fraction, 2.0);
}

double TrajectoryObjective::calculateTimeCost(
    const Trajectory& trajectory,
    const MissionRequest& mission
) const
{
    const double tof =
        trajectory.getTimeOfFlight();

    if (tof < mission.minimumTimeOfFlight)
        return 1e6;

    if (tof > mission.maximumTimeOfFlight)
        return 1e6;

    const double normalized =
        (tof - mission.minimumTimeOfFlight) /
        (
            mission.maximumTimeOfFlight -
            mission.minimumTimeOfFlight
        );

    return normalized;
}

double TrajectoryObjective::calculateReversalPenalty(
    const Trajectory& trajectory
) const
{
    if (trajectory.burns.size() < 2)
        return 0.0;

    double penalty = 0.0;

    for (std::size_t i = 1;
         i < trajectory.burns.size();
         ++i)
    {
        const Burn& previous =
            trajectory.burns[i - 1];

        const Burn& current =
            trajectory.burns[i];

        const double previousDV =
            previous.getDeltaV();

        const double currentDV =
            current.getDeltaV();

        if (previousDV <= 0.0 ||
            currentDV <= 0.0)
        {
            continue;
        }

        const Vec3 a =
            previous.deltaVelocity.normalized();

        const Vec3 b =
            current.deltaVelocity.normalized();

        const double alignment =
            a.dot(b);

        /*
         * Only strongly opposing burns.
         */
        if (alignment > -0.5)
            continue;

        const double gap =
            current.startTime -
            previous.getEndTime();

        /*
         * Opposing burns close together are
         * suspiciously likely to be wasting fuel
         * simply to reduce TOF.
         */
        const double closeness =
            std::exp(
                -gap / 86400.0
            );

        const double magnitude =
            previousDV +
            currentDV;

        penalty +=
            magnitude *
            magnitude *
            closeness *
            (-alignment);
    }

    return penalty;
}

double TrajectoryObjective::calculateBurnDurationPenalty(
    const Trajectory& trajectory
) const
{
    constexpr double preferredBurnDuration =
        30.0 * 60.0;

    double penalty = 0.0;

    for (const Burn& burn : trajectory.burns)
    {
        if (burn.duration <= preferredBurnDuration)
            continue;

        const double ratio =
            burn.duration /
            preferredBurnDuration;

        penalty +=
            0.01 *
            ratio *
            ratio;
    }

    return penalty;
}