//
// Created by tdragon on 9/21/26.
//

#include "TrajectoryResult.h"

#include <cmath>

bool TrajectoryResult::hasFailed() const
{
    return !valid ||
           collided ||
           escaped ||
           !completed;
}

bool TrajectoryResult::isSuccessful() const
{
    return valid &&
           completed &&
           reachedTarget &&
           captured &&
           !collided &&
           !escaped;
}