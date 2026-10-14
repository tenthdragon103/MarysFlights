//
// Created by tdragon on 9/17/26.
//

#ifndef MARYSFLIGHTS_VEC3_H
#define MARYSFLIGHTS_VEC3_H

#pragma once

#include <cmath>

class Vec3
{
public:
    double x;
    double y;
    double z;

    constexpr Vec3()
        : x(0.0), y(0.0), z(0.0)
    {
    }

    constexpr Vec3(
        double x,
        double y,
        double z
    )
        : x(x), y(y), z(z)
    {
    }

    constexpr Vec3 operator+(const Vec3& other) const
    {
        return {
            x + other.x,
            y + other.y,
            z + other.z
        };
    }

    constexpr Vec3 operator-(const Vec3& other) const
    {
        return {
            x - other.x,
            y - other.y,
            z - other.z
        };
    }

    constexpr Vec3 operator*(double scalar) const
    {
        return {
            x * scalar,
            y * scalar,
            z * scalar
        };
    }

    constexpr Vec3 operator/(double scalar) const
    {
        return {
            x / scalar,
            y / scalar,
            z / scalar
        };
    }

    Vec3& operator/=(double scalar)
    {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }

    Vec3& operator+=(const Vec3& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;

        return *this;
    }

    Vec3& operator-=(const Vec3& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;

        return *this;
    }

    Vec3& operator*=(double scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;

        return *this;
    }

    double lengthSquared() const
    {
        return
            x * x +
            y * y +
            z * z;
    }

    double length() const
    {
        return std::sqrt(lengthSquared());
    }

    Vec3 normalized() const
    {
        const double magnitude = length();

        if (magnitude <= 0.0)
            return {};

        return *this / magnitude;
    }

    double dot(const Vec3& other) const
    {
        return
            x * other.x +
            y * other.y +
            z * other.z;
    }

    Vec3 cross(const Vec3& other) const
    {
        return {
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        };
    }
};

constexpr Vec3 operator*(
    double scalar,
    const Vec3& vector
)
{
    return vector * scalar;
}

#endif //MARYSFLIGHTS_VEC3_H
