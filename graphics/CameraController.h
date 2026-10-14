//
// Created by tdragon on 9/17/26.
//

#ifndef MARYSFLIGHTS_CAMERACONTROLLER_H
#define MARYSFLIGHTS_CAMERACONTROLLER_H
#include "raylib.h"


class CameraController
{
public:
    CameraController();

    void update();

    void setFocus(Vector3 position);
    void setDistance(float distance);

    [[nodiscard]] Vector3 getPosition() const;
    [[nodiscard]] Vector3 getTarget() const;

    Camera3D& getCamera();
    [[nodiscard]] const Camera3D& getCamera() const;

private:
    Camera3D camera;

    Vector3 focus;

    float distance;

    float yaw;
    float pitch;

    float rotationSpeed;
    float zoomSpeed;

    void updateCameraPosition();
};


#endif //MARYSFLIGHTS_CAMERACONTROLLER_H
