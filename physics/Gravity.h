//
// Created by tdragon on 9/17/26.
//

#ifndef MARYSFLIGHTS_GRAVITY_H
#define MARYSFLIGHTS_GRAVITY_H


#pragma once

#include "../bodies/Body.h"

class Gravity
{
public:
    static Vec3 accelerationBetween(
        const Body& source,
        const Body& target
    );
};


#endif //MARYSFLIGHTS_GRAVITY_H
