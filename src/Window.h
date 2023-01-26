#pragma once
#include "raylib.h"
#include "CameraController.h"

class Window {
private:
public:
    Window(const char *title, int width, int height);
    ~Window();
    void mainloop();
};