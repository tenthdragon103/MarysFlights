//
// Created by tdragon on 9/17/26.
//

#include "Body.h"

#include <cmath>
#include <stdexcept>

Body::Body(
    const std::string& name,
    double mass,
    double radius,
    double gravitationalParameter
)
    : name(name),
      mass(mass),
      radius(radius),
      gravitationalParameter(gravitationalParameter),
      parentType(ParentType::SystemBarycenter),
      parent(nullptr),
      position(),
      velocity(),
      acceleration(),
      heading(0.0, 0.0, 1.0),
      color{255, 255, 255, 255},
      active(false)
{
}

const std::string& Body::getName() const
{
    return name;
}

double Body::getMass() const
{
    return mass;
}

double Body::getRadius() const
{
    return radius;
}

double Body::getMu() const
{
    return gravitationalParameter;
}

double Body::getSurfaceGravity() const
{
    if (radius <= 0.0)
        return 0.0;

    return gravitationalParameter /
           (radius * radius);
}

double Body::getEscapeVelocity(double altitude) const
{
    const double distance = radius + altitude;

    if (distance <= 0.0)
        return 0.0;

    return std::sqrt(
        2.0 *
        gravitationalParameter /
        distance
    );
}

const Vec3& Body::getPosition() const
{
    return position;
}

const Vec3& Body::getVelocity() const
{
    return velocity;
}

const Vec3& Body::getAcceleration() const
{
    return acceleration;
}

const Vec3& Body::getHeading() const
{
    return heading;
}

void Body::setPosition(const Vec3& value)
{
    position = value;
}

void Body::setVelocity(const Vec3& value)
{
    velocity = value;
}

void Body::setAcceleration(const Vec3& value)
{
    acceleration = value;
}

void Body::setHeading(const Vec3& value)
{
    heading = value;
}

bool Body::isActive() const
{
    return active;
}

void Body::setActive(bool value)
{
    active = value;
}

Body* Body::getParent() const
{
    return parent;
}

void Body::setParent(Body* value)
{
    parent = value;

    if (value != nullptr)
        parentType = ParentType::Body;
}

ParentType Body::getParentType() const
{
    return parentType;
}

void Body::setParentType(ParentType type)
{
    parentType = type;

    if (type == ParentType::SystemBarycenter)
        parent = nullptr;
}

const BodyColor& Body::getColor() const
{
    return color;
}

void Body::setColor(const BodyColor& value)
{
    color = value;
}