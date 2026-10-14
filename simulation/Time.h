//
// Created by tdragon on 9/19/26.
//

#ifndef MARYSFLIGHTS_TIME_H
#define MARYSFLIGHTS_TIME_H

#pragma once

#include <string>

class Time
{
public:
    /*
     * Converts an ISO-8601 calendar date representing
     * TDB into Julian Date.
     *
     * Expected format:
     *
     * YYYY-MM-DDTHH:MM:SS
     *
     * Optional fractional seconds are supported:
     *
     * YYYY-MM-DDTHH:MM:SS.sss
     *
     * The value is interpreted as TDB.
     */
    static double julianDateFromTDB(
        const std::string& isoDate
    );

    /*
     * Returns the number of seconds between
     * two Julian Dates.
     */
    static double secondsBetween(
        double startJulianDate,
        double endJulianDate
    );

    /*
     * Converts a Julian Date back into an ISO
     * calendar representation.
     */
    static std::string tdbFromJulianDate(
        double julianDate
    );
};


#endif //MARYSFLIGHTS_TIME_H
