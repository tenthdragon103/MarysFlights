//
// Created by tdragon on 9/17/26.
//

#ifndef MARYSFLIGHTS_SIMULATION_H
#define MARYSFLIGHTS_SIMULATION_H

#pragma once

#include "../bodies/SolarSystem.h"

#include <string>

class Simulation
{
public:

    Simulation();

    void startNew();

    bool loadSave(
        const std::string& filename
    );

    void update(
        double deltaTime
    );

    [[nodiscard]] bool isRunning() const;

    [[nodiscard]] const SolarSystem& getSolarSystem() const;

    [[nodiscard]] double getSimulationTime() const;

    [[nodiscard]] double getEpochJulianDate() const;
    [[nodiscard]] double getCurrentJulianDate() const;

private:
    bool running;

    double simulationTime;
    double timeScale;

    double accumulator;
    double physicsStep;

    SolarSystem solarSystem;

    double nextDiagnosticTime;

    double epochJulianDate;
};


#endif //MARYSFLIGHTS_SIMULATION_H
