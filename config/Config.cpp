//
// Created by tdragon on 10/13/26.
//


#include "Config.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

std::string Config::bodies_file_;
double Config::timescale_ = 1.0;
double Config::physics_step_ = 1.0;
bool Config::initialized_ = false;

static std::string trim(const std::string& value)
{
    const std::size_t first =
        value.find_first_not_of(" \t\r\n");

    if (first == std::string::npos)
        return "";

    const std::size_t last =
        value.find_last_not_of(" \t\r\n");

    return value.substr(first, last - first + 1);
}

void Config::initialize()
{
    if (initialized_)
        return;

    const std::filesystem::path configPath =
        std::filesystem::current_path() /
        "config" /
        "options.yml";

    std::ifstream file(configPath);

    if (!file)
        throw std::runtime_error(
            "Cannot open config file: " + configPath.string()
        );

    bool foundBodiesFile = false;
    bool foundTimeScale = false;
    bool foundPhysicsStep = false;

    std::string line;
    std::size_t lineNumber = 0;

    while (std::getline(file, line))
    {
        ++lineNumber;

        const std::size_t comment = line.find('#');

        if (comment != std::string::npos)
            line.erase(comment);

        line = trim(line);

        if (line.empty())
            continue;

        const std::size_t separator = line.find(':');

        if (separator == std::string::npos)
            throw std::runtime_error(
                "Malformed config entry at line " +
                std::to_string(lineNumber)
            );

        const std::string key =
            trim(line.substr(0, separator));

        const std::string value =
            trim(line.substr(separator + 1));

        if (key.empty() || value.empty())
            throw std::runtime_error(
                "Missing key or value at line " +
                std::to_string(lineNumber)
            );

        if (key == "bodiesFile")
        {
            if (foundBodiesFile)
                throw std::runtime_error(
                    "Duplicate config option: bodiesFile"
                );

            if (std::filesystem::path(value).is_absolute())
                throw std::runtime_error(
                    "bodiesFile must be relative to config/systems/"
                );

            bodies_file_ = value;
            foundBodiesFile = true;
        }
        else if (key == "timeScale")
        {
            if (foundTimeScale)
                throw std::runtime_error(
                    "Duplicate config option: timeScale"
                );

            timescale_ = std::stod(value);

            if (!std::isfinite(timescale_) || timescale_ <= 0.0)
                throw std::runtime_error(
                    "timeScale must be finite and positive"
                );

            foundTimeScale = true;
        }
        else if (key == "physicsStep")
        {
            if (foundPhysicsStep)
                throw std::runtime_error(
                    "Duplicate config option: physicsStep"
                );

            physics_step_ = std::stod(value);

            if (!std::isfinite(physics_step_) ||
                physics_step_ <= 0.0)
                throw std::runtime_error(
                    "physicsStep must be finite and positive"
                );

            foundPhysicsStep = true;
        }
        else
        {
            throw std::runtime_error(
                "Unknown config option at line " +
                std::to_string(lineNumber) + ": " + key
            );
        }
    }

    if (!file.eof() && file.fail())
        throw std::runtime_error(
            "Error reading config file: " + configPath.string()
        );

    if (!foundBodiesFile ||
        !foundTimeScale ||
        !foundPhysicsStep)
    {
        std::string missing;

        if (!foundBodiesFile)
            missing += " bodiesFile";
        if (!foundTimeScale)
            missing += " timeScale";
        if (!foundPhysicsStep)
            missing += " physicsStep";

        throw std::runtime_error(
            "Missing required config options:" + missing
        );
    }

    initialized_ = true;
}

std::string Config::getBodiesFile()
{
    if (!initialized_)
        throw std::runtime_error(
            "Config::initialize() must be called first"
        );

    static const std::filesystem::path systemsDirectory =
        std::filesystem::current_path() /
        "config" /
        "systems";

    static std::string path;

    path = (systemsDirectory / bodies_file_).string();
    return path;
}

double Config::getTimeScale()
{
    if (!initialized_)
        throw std::runtime_error(
            "Config::initialize() must be called first"
        );

    return timescale_;
}

double Config::getPhysicsStep()
{
    if (!initialized_)
        throw std::runtime_error(
            "Config::initialize() must be called first"
        );

    return physics_step_;
}
