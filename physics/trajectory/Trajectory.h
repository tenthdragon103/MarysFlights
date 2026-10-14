//
// Created by tdragon on 9/20/26.
//

#ifndef MARYSFLIGHTS_TRAJECTORY_H
#define MARYSFLIGHTS_TRAJECTORY_H


#pragma once

#include "Burn.h"

#include <vector>

struct Trajectory
{
    std::vector<Burn> burns;

    double startTime = 0.0;
    double endTime = 0.0;

    void addBurn(const Burn& burn);

    void sortBurns();

    [[nodiscard]] double getTimeOfFlight() const;

    [[nodiscard]] double getTotalDeltaV() const;

    [[nodiscard]] double getBurnDuration() const;

    [[nodiscard]] bool isValid() const;
};


#endif //MARYSFLIGHTS_TRAJECTORY_H
