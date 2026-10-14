cmake_minimum_required(VERSION 4.3)
project(MarysFlights)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
)

FetchContent_MakeAvailable(raylib)

add_executable(MarysFlights main.cpp
        simulation/Simulation.cpp
        simulation/Simulation.h
        math/Vec3.cpp
        math/Vec3.h
        bodies/Body.h
        physics/Gravity.cpp
        physics/Gravity.h
        physics/Acceleration.cpp
        physics/Acceleration.h
        physics/Integrator.cpp
        physics/Integrator.h
        bodies/SolarSystem.cpp
        bodies/SolarSystem.h
        bodies/Body.cpp
        graphics/CameraController.cpp
        graphics/CameraController.h
        graphics/ObjectPicker.h
        graphics/ObjectPicker.cpp
        graphics/Renderer.cpp
        graphics/Renderer.h
        physics/State.cpp
        physics/State.h
        simulation/InitialState.cpp
        simulation/InitialState.h
        physics/OrbitalElements.cpp
        physics/OrbitalElements.h
        config/IniFile.cpp
        config/IniFile.h
        simulation/Time.cpp
        simulation/Time.h
        physics/Diagnostics.cpp
        physics/Diagnostics.h
        config/ColorParser.cpp
        config/ColorParser.h
        physics/trajectory/Burn.h
        physics/trajectory/Trajectory.cpp
        physics/trajectory/Trajectory.h
        physics/trajectory/MissionRequest.h
        optimization/TrajectoryObjective.cpp
        optimization/TrajectoryObjective.h
        optimization/BurnTimeRefiner.cpp
        optimization/BurnTimeRefiner.h
        physics/trajectory/TrajectoryResult.cpp
        physics/trajectory/TrajectoryResult.h
        agent/Integrator.cpp
        agent/Integrator.h
)

target_link_libraries(MarysFlights PRIVATE raylib)