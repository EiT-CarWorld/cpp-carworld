#pragma once
#include "raylib.h"

class CameraController {
private:
    Camera3D m_camera;

    // in radians
    float m_yaw; // 0 is towards -z, positive angle towards +x
    float m_pitch; //pi/2 is +y (up), -pi/2 is -y (down)

    // The factor to scale camera speed with when holding shift
    float m_cameraSpeedShiftFactor{2.f};
public:
    CameraController();
    ~CameraController();
    void resetCamera(Vector3 position);
    void updateCamera();
    Camera3D getCamera();
};