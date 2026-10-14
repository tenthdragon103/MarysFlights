//
// Created by tdragon on 9/20/26.
//

#include "ColorParser.h"

#include <algorithm>
#include <stdexcept>

namespace
{
    std::string lower(
        std::string value
    )
    {
        std::transform(
            value.begin(),
            value.end(),
            value.begin(),
            [](unsigned char c)
            {
                return static_cast<char>(
                    std::tolower(c)
                );
            }
        );

        return value;
    }
}

BodyColor ColorParser::parse(
    const std::string& value
)
{
    const std::string color =
        lower(value);

    if (color == "gray")
        return {128, 128, 128, 255};

    if (color == "darkgray")
        return {64, 64, 64, 255};

    if (color == "lightgray")
        return {192, 192, 192, 255};

    if (color == "white")
        return {255, 255, 255, 255};

    if (color == "black")
        return {0, 0, 0, 255};

    if (color == "red")
        return {255, 0, 0, 255};

    if (color == "green")
        return {0, 255, 0, 255};

    if (color == "blue")
        return {0, 0, 255, 255};

    if (color == "darkblue")
        return {100, 0, 255, 255};

    if (color == "skyblue")
        return {135, 206, 235, 255};

    if (color == "yellow")
        return {255, 255, 0, 255};

    if (color == "orange")
        return {255, 165, 0, 255};

    if (color == "gold")
        return {255, 215, 0, 255};

    if (color == "cyan")
        return {0, 255, 255, 255};

    if (color == "purple")
        return {128, 0, 128, 255};

    if (color == "brown")
        return {139, 69, 19, 255};

    if (color == "darkbrown")
        return {101, 67, 33, 255};

    if (color == "maroon")
        return {128, 0, 0, 255};

    if (color == "beige")
        return {245, 245, 220, 255};

    throw std::runtime_error(
        "Unknown body color: " +
        value
    );
}