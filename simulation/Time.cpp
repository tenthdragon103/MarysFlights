//
// Created by tdragon on 9/19/26.
//

#include "Time.h"

#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace
{
    constexpr double SECONDS_PER_DAY =
        86400.0;

    struct DateTime
    {
        int year;
        int month;
        int day;

        int hour;
        int minute;

        double second;
    };

    DateTime parseISO8601(
        const std::string& value
    )
    {
        if (value.size() < 19)
        {
            throw std::invalid_argument(
                "Invalid TDB date: " + value
            );
        }

        if (value[4] != '-' ||
            value[7] != '-' ||
            value[10] != 'T' ||
            value[13] != ':' ||
            value[16] != ':')
        {
            throw std::invalid_argument(
                "Invalid TDB date format: " +
                value
            );
        }

        try
        {
            DateTime date{};

            date.year =
                std::stoi(
                    value.substr(0, 4)
                );

            date.month =
                std::stoi(
                    value.substr(5, 2)
                );

            date.day =
                std::stoi(
                    value.substr(8, 2)
                );

            date.hour =
                std::stoi(
                    value.substr(11, 2)
                );

            date.minute =
                std::stoi(
                    value.substr(14, 2)
                );

            date.second =
                std::stod(
                    value.substr(17)
                );

            if (date.month < 1 ||
                date.month > 12)
            {
                throw std::invalid_argument(
                    "Invalid month"
                );
            }

            if (date.day < 1 ||
                date.day > 31)
            {
                throw std::invalid_argument(
                    "Invalid day"
                );
            }

            if (date.hour < 0 ||
                date.hour > 23)
            {
                throw std::invalid_argument(
                    "Invalid hour"
                );
            }

            if (date.minute < 0 ||
                date.minute > 59)
            {
                throw std::invalid_argument(
                    "Invalid minute"
                );
            }

            if (date.second < 0.0 ||
                date.second >= 60.0)
            {
                throw std::invalid_argument(
                    "Invalid second"
                );
            }

            return date;
        }
        catch (const std::exception&)
        {
            throw std::invalid_argument(
                "Invalid TDB date: " + value
            );
        }
    }

    double julianDateFromCalendar(
        const DateTime& date
    )
    {
        /*
         * Fliegel-Van Flandern style
         * Gregorian Julian Date calculation.
         *
         * We keep the fractional day separately
         * to preserve precision.
         */

        int year = date.year;
        int month = date.month;

        if (month <= 2)
        {
            --year;
            month += 12;
        }

        const int A =
            year / 100;

        const int B =
            2 -
            A +
            A / 4;

        const double integerDay =
            std::floor(
                365.25 *
                (year + 4716)
            ) +
            std::floor(
                30.6001 *
                (month + 1)
            ) +
            date.day +
            B -
            1524.5;

        const double secondsSinceMidnight =
            static_cast<double>(date.hour) *
                3600.0 +
            static_cast<double>(date.minute) *
                60.0 +
            date.second;

        const double fractionalDay =
            secondsSinceMidnight /
            SECONDS_PER_DAY;

        return integerDay +
               fractionalDay;
    }

    void calendarFromJulianDate(
        double julianDate,
        int& year,
        int& month,
        int& day,
        int& hour,
        int& minute,
        double& second
    )
    {
        /*
         * Convert JD to Gregorian calendar date.
         */

        const double shifted =
            julianDate + 0.5;

        const double Z =
            std::floor(shifted);

        const double F =
            shifted - Z;

        double A = Z;

        if (Z >= 2299161.0)
        {
            const double alpha =
                std::floor(
                    (Z - 1867216.25) /
                    36524.25
                );

            A =
                Z +
                1.0 +
                alpha -
                std::floor(alpha / 4.0);
        }

        const double B =
            A + 1524.0;

        const double C =
            std::floor(
                (B - 122.1) /
                365.25
            );

        const double D =
            std::floor(
                365.25 * C
            );

        const double E =
            std::floor(
                (B - D) /
                30.6001
            );

        const double dayWithFraction =
            B -
            D -
            std::floor(
                30.6001 * E
            ) +
            F;

        day =
            static_cast<int>(
                std::floor(
                    dayWithFraction
                )
            );

        if (E < 14.0)
            month =
                static_cast<int>(E - 1.0);
        else
            month =
                static_cast<int>(E - 13.0);

        if (month > 2)
            year =
                static_cast<int>(
                    C - 4716.0
                );
        else
            year =
                static_cast<int>(
                    C - 4715.0
                );

        const double fractionalDay =
            dayWithFraction -
            static_cast<double>(day);

        double totalSeconds =
            fractionalDay *
            SECONDS_PER_DAY;

        hour =
            static_cast<int>(
                totalSeconds / 3600.0
            );

        totalSeconds -=
            static_cast<double>(hour) *
            3600.0;

        minute =
            static_cast<int>(
                totalSeconds / 60.0
            );

        second =
            totalSeconds -
            static_cast<double>(minute) *
            60.0;

        /*
         * Protect against floating point values
         * such as 59.999999999 becoming 60.0.
         */
        if (second >= 59.9999999995)
        {
            second = 0.0;
            ++minute;

            if (minute >= 60)
            {
                minute = 0;
                ++hour;

                if (hour >= 24)
                {
                    hour = 0;

                    /*
                     * Recalculate using the next
                     * Julian day.
                     */
                    calendarFromJulianDate(
                        julianDate +
                        1.0,
                        year,
                        month,
                        day,
                        hour,
                        minute,
                        second
                    );
                }
            }
        }
    }
}

double Time::julianDateFromTDB(
    const std::string& isoDate
)
{
    const DateTime date =
        parseISO8601(isoDate);

    return julianDateFromCalendar(
        date
    );
}

double Time::secondsBetween(
    double startJulianDate,
    double endJulianDate
)
{
    return (
        endJulianDate -
        startJulianDate
    ) * SECONDS_PER_DAY;
}

std::string Time::tdbFromJulianDate(
    double julianDate
)
{
    int year;
    int month;
    int day;
    int hour;
    int minute;

    double second;

    calendarFromJulianDate(
        julianDate,
        year,
        month,
        day,
        hour,
        minute,
        second
    );

    std::ostringstream output;

    output
        << std::setfill('0')
        << std::setw(4)
        << year
        << '-'
        << std::setw(2)
        << month
        << '-'
        << std::setw(2)
        << day
        << 'T'
        << std::setw(2)
        << hour
        << ':'
        << std::setw(2)
        << minute
        << ':'
        << std::fixed
        << std::setprecision(6)
        << std::setw(9)
        << second;

    return output.str();
}