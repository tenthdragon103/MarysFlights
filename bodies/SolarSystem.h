//
// Created by tdragon on 9/17/26.
//

#ifndef MARYSFLIGHTS_SOLARSYSTEM_H
#define MARYSFLIGHTS_SOLARSYSTEM_H


#pragma once

#include "Body.h"

#include <memory>
#include <string>
#include <vector>

struct BarycentricState
{
    Vec3 position;
    Vec3 velocity;
};

class SolarSystem
{
public:
    SolarSystem();
    
    Body& createBody(
        const std::string& name,
        double mass,
        double radius,
        double mu
    );

    Body* findBody(
        const std::string& name
    );

    const Body* findBody(
        const std::string& name
    ) const;

    void clear();

    const std::vector<std::unique_ptr<Body>>&
    getBodies() const;

    std::vector<Body*> getBodyPointers() const;

    const BarycentricState& getBarycenter() const;

    void setBarycenter(
        const BarycentricState& state
    );


private:
    std::vector<std::unique_ptr<Body>> bodies;
    BarycentricState barycenter;
};

#endif //MARYSFLIGHTS_SOLARSYSTEM_H
