//
// Created by tdragon on 9/19/26.
//

#include "IniFile.h"
#include "IniFile.h"

#include <fstream>
#include <ranges>
#include <sstream>
#include <stdexcept>

namespace
{
    std::string trim(
        const std::string& value
    )
    {
        const auto begin =
            value.find_first_not_of(
                " \t\r\n"
            );

        if (begin ==
            std::string::npos)
        {
            return {};
        }

        const auto end =
            value.find_last_not_of(
                " \t\r\n"
            );

        return value.substr(
            begin,
            end - begin + 1
        );
    }
}

bool IniFile::load(
    const std::string& filename
)
{
    sections.clear();

    std::ifstream file(filename);

    if (!file)
        return false;

    std::string currentSection;
    std::string line;

    while (std::getline(file, line))
    {
        line = trim(line);

        if (line.empty())
            continue;

        if (line[0] == '#' ||
            line[0] == ';')
        {
            continue;
        }

        if (line.front() == '[' &&
            line.back() == ']')
        {
            currentSection =
                trim(
                    line.substr(
                        1,
                        line.size() - 2
                    )
                );

            continue;
        }

        const std::size_t equals =
            line.find('=');

        if (equals ==
            std::string::npos)
        {
            continue;
        }

        const std::string key =
            trim(
                line.substr(
                    0,
                    equals
                )
            );

        const std::string value =
            trim(
                line.substr(
                    equals + 1
                )
            );

        sections[currentSection][key] =
            value;
    }

    return true;
}

bool IniFile::has(
    const std::string& section,
    const std::string& key
) const
{
    const auto sectionIt =
        sections.find(section);

    if (sectionIt ==
        sections.end())
    {
        return false;
    }

    return sectionIt->second.contains(key);
}

std::string IniFile::getString(
    const std::string& section,
    const std::string& key
) const
{
    const auto sectionIt =
        sections.find(section);

    if (sectionIt ==
        sections.end())
    {
        throw std::runtime_error(
            "Missing INI section: " +
            section
        );
    }

    const auto keyIt =
        sectionIt->second.find(key);

    if (keyIt ==
        sectionIt->second.end())
    {
        throw std::runtime_error(
            "Missing INI key: " +
            section +
            "." +
            key
        );
    }

    return keyIt->second;
}

double IniFile::getDouble(
    const std::string& section,
    const std::string& key
) const
{
    return std::stod(
        getString(
            section,
            key
        )
    );
}

std::vector<std::string>
IniFile::getSections() const
{
    std::vector<std::string> result;

    for (const auto &name: sections | std::views::keys)
    {
        result.push_back(name);
    }

    return result;
}