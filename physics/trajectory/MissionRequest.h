//
// Created by tdragon on 9/20/26.
//

#ifndef MARYSFLIGHTS_MISSIONREQUEST_H
#define MARYSFLIGHTS_MISSIONREQUEST_H


#pragma once

#include <string>
#include <vector>

struct MissionRequest
{
    std::string departureBody;
    std::string targetBody;

    /*
     * Bodies that MUST be visited.
     */
    std::vector<std::string> requiredBodies;

    /*
     * Bodies that may not be used.
     */
    std::vector<std::string> deniedBodies;

    /*
     * Maximum available total delta-v.
     */
    double deltaVBudget = 0.0;

    /*
     * Desired limits.
     */
    double minimumTimeOfFlight = 0.0;
    double maximumTimeOfFlight = 0.0;

    /*
     * Whether the target must be captured
     * rather than merely reached.
     */
    bool requireCapture = true;
};


#endif //MARYSFLIGHTS_MISSIONREQUEST_H
