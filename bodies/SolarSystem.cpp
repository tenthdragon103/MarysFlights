//
// Created by tdragon on 9/17/26.
//

#include "SolarSystem.h"

#include <stdexcept>

SolarSystem::SolarSystem()
    : barycenter{}
{
}

Body& SolarSystem::createBody(
    const std::string& name,
    double mass,
    double radius,
    double mu
)
{
    if (findBody(name) != nullptr)
    {
        throw std::runtime_error(
            "Body already exists: " + name
        );
    }

    auto body =
        std::make_unique<Body>(
            name,
            mass,
            radius,
            mu
        );

    Body& reference = *body;

    bodies.push_back(
        std::move(body)
    );

    return reference;
}

Body* SolarSystem::findBody(
    const std::string& name
)
{
    for (const auto& body : bodies)
    {
        if (body->getName() == name)
            return body.get();
    }

    return nullptr;
}

const Body* SolarSystem::findBody(
    const std::string& name
) const
{
    for (const auto& body : bodies)
    {
        if (body->getName() == name)
            return body.get();
    }

    return nullptr;
}

void SolarSystem::clear()
{
    bodies.clear();
}

const std::vector<std::unique_ptr<Body>>&
SolarSystem::getBodies() const
{
    return bodies;
}

std::vector<Body*> SolarSystem::getBodyPointers() const
{
    std::vector<Body*> result;

    result.reserve(bodies.size());

    for (const auto& body : bodies)
    {
        result.push_back(body.get());
    }

    return result;
}

const BarycentricState&
SolarSystem::getBarycenter() const
{
    return barycenter;
}

void SolarSystem::setBarycenter(
    const BarycentricState& state
)
{
    barycenter = state;
}