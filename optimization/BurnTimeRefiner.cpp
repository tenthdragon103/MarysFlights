//
// Created by tdragon on 9/21/26.
//

#include "BurnTimeRefiner.h"

#include <algorithm>
#include <limits>

Trajectory BurnTimeRefiner::refine(
    const Trajectory& initial,
    const MissionRequest& mission,
    const EvaluateFunction& evaluate
) const
{
    Trajectory best = initial;

    best.sortBurns();

    double bestScore =
        evaluate(best);

    /*
     * Start with relatively coarse timing adjustments.
     */
    double step =
        7.0 * 86400.0;

    constexpr double minimumStep =
        60.0;

    constexpr double minimumBurnGap =
        60.0 * 60.0;

    while (step >= minimumStep)
    {
        bool improved = true;

        while (improved)
        {
            improved = false;

            for (std::size_t i = 0;
                 i < best.burns.size();
                 ++i)
            {
                if (tryImproveBurn(
                        best,
                        i,
                        step,
                        minimumBurnGap,
                        evaluate))
                {
                    bestScore =
                        evaluate(best);

                    improved = true;
                }
            }
        }

        step *= 0.5;
    }

    return best;
}

bool BurnTimeRefiner::tryImproveBurn(
    Trajectory& trajectory,
    std::size_t burnIndex,
    double step,
    double minimumGap,
    const EvaluateFunction& evaluate
) const
{
    if (burnIndex >= trajectory.burns.size())
        return false;

    const double originalTime =
        trajectory.burns[burnIndex].startTime;

    const double originalScore =
        evaluate(trajectory);

    double bestTime =
        originalTime;

    double bestScore =
        originalScore;

    const double candidates[] =
    {
        originalTime - step,
        originalTime + step
    };

    for (const double candidateTime : candidates)
    {
        if (candidateTime <
            trajectory.startTime)
        {
            continue;
        }

        if (candidateTime >
            trajectory.endTime)
        {
            continue;
        }

        trajectory.burns[burnIndex].startTime =
            candidateTime;

        trajectory.sortBurns();

        bool valid = true;

        for (std::size_t i = 1;
             i < trajectory.burns.size();
             ++i)
        {
            const Burn& previous =
                trajectory.burns[i - 1];

            const Burn& current =
                trajectory.burns[i];

            if (current.startTime <
                previous.getEndTime() +
                minimumGap)
            {
                valid = false;
                break;
            }
        }

        if (!valid)
        {
            trajectory.burns[burnIndex].startTime =
                originalTime;

            trajectory.sortBurns();

            continue;
        }

        const double score =
            evaluate(trajectory);

        if (score < bestScore)
        {
            bestScore = score;
            bestTime = candidateTime;
        }

        trajectory.burns[burnIndex].startTime =
            originalTime;

        trajectory.sortBurns();
    }

    if (bestTime != originalTime)
    {
        trajectory.burns[burnIndex].startTime =
            bestTime;

        trajectory.sortBurns();

        return true;
    }

    return false;
}