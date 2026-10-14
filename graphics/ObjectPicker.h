//
// Created by tdragon on 9/17/26.
//

#ifndef MARYSFLIGHTS_OBJECTPICKER_H
#define MARYSFLIGHTS_OBJECTPICKER_H

#pragma once

#include "../bodies/Body.h"
#include "../bodies/SolarSystem.h"
#include "raylib.h"

class ObjectPicker
{
public:
    static const Body* pickBody(
        const Camera3D& camera,
        const SolarSystem& solarSystem,
        double solarScale,
        double minimumRadius
    );

private:
    static Ray getMouseRay(
        const Camera3D& camera
    );

    static bool intersectsSphere(
        const Ray& ray,
        const Vector3& center,
        float radius,
        float& distance
    );
};


#endif //MARYSFLIGHTS_OBJECTPICKER_H
