//
// Created by tdragon on 9/20/26.
//

#ifndef MARYSFLIGHTS_COLORPARSER_H
#define MARYSFLIGHTS_COLORPARSER_H


#pragma once

#include "../bodies/Body.h"

#include <string>

class ColorParser
{
public:
    static BodyColor parse(
        const std::string& value
    );
};


#endif //MARYSFLIGHTS_COLORPARSER_H
