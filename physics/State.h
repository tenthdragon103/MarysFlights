//
// Created by tdragon on 9/18/26.
//

#ifndef MARYSFLIGHTS_STATE_H
#define MARYSFLIGHTS_STATE_H

#pragma once

#include "../math/Vec3.h"

struct State
{
    Vec3 position;
    Vec3 velocity;
    Vec3 acceleration;

    Vec3 heading;

    bool active = true;
};


#endif //MARYSFLIGHTS_STATE_H
