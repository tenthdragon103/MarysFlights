//
// Created by tdragon on 9/17/26.
//

#include "Simulation.h"

#include <filesystem>
#include <iostream>

#include "InitialState.h"
#include "raylib.h"
#include "../physics/Integrator.h"
#include "../physics/Acceleration.h"
#include "../physics/Diagnostics.h"
#include "../config/Config.h"

Simulation::Simulation()
    : running(false),
      simulationTime(0.0),
      timeScale(Config::getTimeScale()),
      accumulator(0.0),
      physicsStep(Config::getPhysicsStep()),
      nextDiagnosticTime(0.0),
      epochJulianDate(0.0)
{
}

void Simulation::startNew()
{
    simulationTime = 0.0;
    accumulator = 0.0;
    nextDiagnosticTime = 0.0;
    
    epochJulianDate =
        InitialState::initialize(
            solarSystem,
            Config::getBodiesFile()
        );

    Acceleration::calculate(
        solarSystem
    );

    const SystemDiagnostics diagnostics =
    Diagnostics::calculate(
        solarSystem
    );

    std::cout
        << "===== INITIAL SYSTEM =====\n"
        << "COM position: "
        << diagnostics.centerOfMass.x
        << ", "
        << diagnostics.centerOfMass.y
        << ", "
        << diagnostics.centerOfMass.z
        << " km\n"

        << "COM velocity: "
        << diagnostics.centerOfMassVelocity.x
        << ", "
        << diagnostics.centerOfMassVelocity.y
        << ", "
        << diagnostics.centerOfMassVelocity.z
        << " km/s\n"

        << "Total momentum: "
        << diagnostics.totalMomentum.x
        << ", "
        << diagnostics.totalMomentum.y
        << ", "
        << diagnostics.totalMomentum.z
        << " kg km/s\n"

        << "Total energy: "
        << diagnostics.totalEnergy
        << " km^2 kg/s^2\n"

        << std::endl;

    running = true;
}

bool Simulation::loadSave(
    const std::string& filename
)
{
    // Save system will be implemented later.

    (void)filename;

    return false;
}

void Simulation::update(
    double realDeltaTime
)
{
    if (!running)
        return;

    accumulator +=
        realDeltaTime *
        timeScale;

    /*
     * Prevent an enormous backlog if the
     * application is paused/debugged.
     */
    constexpr double maxAccumulatedTime = 100000.0;

    if (accumulator > maxAccumulatedTime)
        accumulator = maxAccumulatedTime;

    while (accumulator >= physicsStep)
    {
        Integrator::step(
            solarSystem,
            physicsStep
        );

        simulationTime += physicsStep;

        accumulator -= physicsStep;

        if (simulationTime >= nextDiagnosticTime)
        {
            TraceLog(
                LOG_INFO,
                "Day %.2f",
                simulationTime / 86400.0
            );

            const SystemDiagnostics diagnostics =
                Diagnostics::calculate(
                    solarSystem
                );

            std::cout
                << "T+"
                << simulationTime
                << " s | "
                << "COM = ("
                << diagnostics.centerOfMass.x
                << ", "
                << diagnostics.centerOfMass.y
                << ", "
                << diagnostics.centerOfMass.z
                << ") km | "
                << "COM velocity = "
                << diagnostics.centerOfMassVelocity.length()
                << " km/s | "
                << "Energy = "
                << diagnostics.totalEnergy
                << '\n';

            nextDiagnosticTime += 86400.0 * 30;
        }
    }
}

bool Simulation::isRunning() const
{
    return running;
}

const SolarSystem&
Simulation::getSolarSystem() const
{
    return solarSystem;
}

double Simulation::getSimulationTime() const
{
    return simulationTime;
}