//
// Created by tdragon on 9/17/26.
//

#ifndef MARYSFLIGHTS_BODY_H
#define MARYSFLIGHTS_BODY_H


#pragma once

#include "../math/Vec3.h"

#include <string>

struct BodyColor
{
    unsigned char r = 255;
    unsigned char g = 255;
    unsigned char b = 255;
    unsigned char a = 255;
};

enum class ParentType
{
    SystemBarycenter,
    Body
};

class Body
{
public:
    Body(
        const std::string& name,
        double mass,
        double radius,
        double gravitationalParameter
    );

    virtual ~Body() = default;

    const std::string& getName() const;

    double getMass() const;
    double getRadius() const;
    double getMu() const;

    double getSurfaceGravity() const;
    double getEscapeVelocity(double altitude = 0.0) const;

    const Vec3& getPosition() const;
    const Vec3& getVelocity() const;
    const Vec3& getAcceleration() const;
    const Vec3& getHeading() const;

    void setPosition(const Vec3& value);
    void setVelocity(const Vec3& value);
    void setAcceleration(const Vec3& value);
    void setHeading(const Vec3& value);

    bool isActive() const;
    void setActive(bool value);

    Body* getParent() const;
    void setParent(Body* value);

    ParentType getParentType() const;
    void setParentType(ParentType type);

    const BodyColor& getColor() const;
    void setColor(const BodyColor& value);

private:
    std::string name;

    double mass;
    double radius;
    double gravitationalParameter;

    ParentType parentType;
    Body* parent;

    Vec3 position;
    Vec3 velocity;
    Vec3 acceleration;
    Vec3 heading;

    BodyColor color;

    bool active;
};


#endif //MARYSFLIGHTS_BODY_H
