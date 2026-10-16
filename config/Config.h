//
// Created by tdragon on 10/13/26.
//

#ifndef MARYSFLIGHTS_CONFIG_H
#define MARYSFLIGHTS_CONFIG_H
#include <string>


class Config
{
public:
    static void initialize();

    static std::string getBodiesFile();
    static double getTimeScale();
    static double getPhysicsStep();

private:
    static std::string bodies_file_;
    static double timescale_;
    static double physics_step_;
    static bool initialized_;
};


#endif //MARYSFLIGHTS_CONFIG_H
