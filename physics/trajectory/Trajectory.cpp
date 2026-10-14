//
// Created by tdragon on 9/20/26.
//

#include "Trajectory.h"

#include <algorithm>
#include <cmath>

void Trajectory::addBurn(
    const Burn& burn
)
{
    burns.push_back(burn);
}

void Trajectory::sortBurns()
{
    std::sort(
        burns.begin(),
        burns.end(),
        [](const Burn& a, const Burn& b)
        {
            return a.startTime < b.startTime;
        }
    );
}

double Trajectory::getTimeOfFlight() const
{
    return endTime - startTime;
}

double Trajectory::getTotalDeltaV() const
{
    double total = 0.0;

    for (const Burn& burn : burns)
        total += burn.getDeltaV();

    return total;
}

double Trajectory::getBurnDuration() const
{
    double total = 0.0;

    for (const Burn& burn : burns)
        total += burn.duration;

    return total;
}

bool Trajectory::isValid() const
{
    if (endTime <= startTime)
        return false;

    for (const Burn& burn : burns)
    {
        if (burn.startTime < startTime)
            return false;

        if (burn.getEndTime() > endTime)
            return false;

        if (burn.duration < 0.0)
            return false;
    }

    return true;
}