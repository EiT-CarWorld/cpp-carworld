#include "CameraController.h"
#include "carMath.h"

CameraController::CameraController() : m_camera({0}), m_mouseLock(false), m_pitch(0), m_yaw(0) {

}

CameraController::~CameraController() {

}

void CameraController::resetCamera(Vector3 position) {
    m_camera.position = position;
    m_camera.target = Vector3{ 0.0f, 0.0f, 0.0f };
    m_camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
    m_camera.fovy = 80.0f;
    m_camera.projection = CAMERA_PERSPECTIVE;
}

void CameraController::updateCamera() {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        lockMouse();
    if (IsKeyPressed(KEY_ESCAPE) || !IsWindowFocused())
        unlockMouse();

    float mouseSensitivity = 0.005;

    if (m_mouseLock) {
        Vector2 delta = GetMouseDelta();
        m_yaw += delta.x * mouseSensitivity;
        m_pitch -= delta.y * mouseSensitivity;
        m_pitch = fmin(PI/2, fmax(-PI/2, m_pitch));
    }

    float cameraSpeed = IsKeyDown(KEY_LEFT_SHIFT) ? 10: 5;
    cameraSpeed *= GetFrameTime();
    Vector3 forwards = {sin(m_yaw) * cos(m_pitch), sin(m_pitch), -cos(m_yaw) * cos(m_pitch)};
    Vector3 xz_forwards = {sin(m_yaw), 0, -cos(m_yaw)};
    Vector3 right = {cos(m_yaw), 0, sin(m_yaw)};
    m_camera.up = Vector3CrossProduct(right, forwards);
    if (IsKeyDown(KEY_W))
        m_camera.position += cameraSpeed * xz_forwards;
    if (IsKeyDown(KEY_S))
        m_camera.position -= cameraSpeed * xz_forwards;
    if (IsKeyDown(KEY_D))
        m_camera.position += cameraSpeed * right;
    if (IsKeyDown(KEY_A))
        m_camera.position -= cameraSpeed * right;
    if (IsKeyDown(KEY_SPACE))
        m_camera.position.y += cameraSpeed;
    if (IsKeyDown(KEY_E))
        m_camera.position.y -= cameraSpeed;

    m_camera.target = m_camera.position + forwards;
}

void CameraController::lockMouse() {
    if (m_mouseLock)
        return;
    DisableCursor();
    m_mouseLock = true;
}

void CameraController::unlockMouse() {
    if (!m_mouseLock)
        return;
    EnableCursor();
    m_mouseLock = false;
}

Camera3D CameraController::getCamera() {
    return m_camera;
}