//
// Created by tdragon on 9/18/26.
//

#ifndef MARYSFLIGHTS_INITIALSTATE_H
#define MARYSFLIGHTS_INITIALSTATE_H

#pragma once

#include "../bodies/SolarSystem.h"
#include "../config/IniFile.h"
#include "../physics/OrbitalElements.h"

#include <string>
#include <unordered_map>

class InitialState
{
public:
    static double initialize(
        SolarSystem& solarSystem,
        const std::string& filename
    );

private:
    static void createBodiesFromIni(
        SolarSystem& solarSystem,
        const IniFile& ini
    );

    static void resolveParents(
        SolarSystem& solarSystem,
        const IniFile& ini
    );

    static void initializeBody(
        SolarSystem& solarSystem,
        Body& body,
        const IniFile& ini,
        std::unordered_map<std::string, bool>& initialized,
        std::unordered_map<std::string, bool>& initializing
    );

    static OrbitalElements readOrbitalElements(
        const IniFile& ini,
        const Body& body
    );

    static void makeBarycentric(
        SolarSystem& solarSystem
    );
};

#endif //MARYSFLIGHTS_INITIALSTATE_H
