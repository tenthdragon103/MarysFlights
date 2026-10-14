//
// Created by tdragon on 9/19/26.
//

#ifndef MARYSFLIGHTS_INIFILE_H
#define MARYSFLIGHTS_INIFILE_H


#pragma once

#include <map>
#include <string>
#include <vector>

class IniFile
{
public:
    bool load(
        const std::string& filename
    );

    std::string getString(
        const std::string& section,
        const std::string& key
    ) const;

    double getDouble(
        const std::string& section,
        const std::string& key
    ) const;

    bool has(
        const std::string& section,
        const std::string& key
    ) const;

    std::vector<std::string> getSections() const;

private:
    using Section =
        std::map<
            std::string,
            std::string
        >;

    std::map<
        std::string,
        Section
    > sections;
};


#endif //MARYSFLIGHTS_INIFILE_H
