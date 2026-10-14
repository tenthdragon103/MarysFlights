//
// Created by tdragon on 9/17/26.
//

#include "CameraController.h"

#include <cmath>

#include "raylib.h"
#include "raymath.h"

CameraController::CameraController()
{
    camera = {};

    camera.position = {
        0.0f,
        5.0f,
        20.0f
    };

    camera.target = {
        0.0f,
        0.0f,
        0.0f
    };

    camera.up = {
        0.0f,
        1.0f,
        0.0f
    };

    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    focus = {
        0.0f,
        0.0f,
        0.0f
    };

    distance = 20.0f;

    yaw = 0.0f;

    pitch = 0.35f;

    rotationSpeed = 0.005f;
    zoomSpeed = 0.15f;
}

void CameraController::update()
{
    // Orbit camera
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        Vector2 delta = GetMouseDelta();

        yaw -= delta.x * rotationSpeed;
        pitch += delta.y * rotationSpeed;

        constexpr float maxPitch = 1.5f;

        if (pitch > maxPitch)
            pitch = maxPitch;

        if (pitch < -maxPitch)
            pitch = -maxPitch;
    }

    // Zoom camera
    float wheel = GetMouseWheelMove();

    if (wheel != 0.0f)
    {
        distance *=
            std::pow(
                1.0f - zoomSpeed,
                wheel
            );
    }

    if (distance < 0.1f)
        distance = 0.1f;

    if (distance > 1.0e9f)
        distance = 1.0e9f;

    updateCameraPosition();
}

void CameraController::setFocus(Vector3 position)
{
    focus = position;

    updateCameraPosition();
}

void CameraController::setDistance(float newDistance)
{
    distance = newDistance;

    if (distance < 0.1f)
        distance = 0.1f;

    updateCameraPosition();
}

void CameraController::updateCameraPosition()
{
    const float cosPitch = std::cos(pitch);
    const float sinPitch = std::sin(pitch);

    const float sinYaw = std::sin(yaw);
    const float cosYaw = std::cos(yaw);

    camera.position = {
        focus.x + distance * cosPitch * sinYaw,
        focus.y + distance * sinPitch,
        focus.z + distance * cosPitch * cosYaw
    };

    camera.target = focus;
}

Vector3 CameraController::getPosition() const
{
    return camera.position;
}

Vector3 CameraController::getTarget() const
{
    return camera.target;
}

Camera3D& CameraController::getCamera()
{
    return camera;
}

const Camera3D& CameraController::getCamera() const
{
    return camera;
}