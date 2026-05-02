#pragma once

struct CameraMovement {
    bool Forward = false;
    bool Backward = false;
    bool StrafeLeft = false;
    bool StrafeRight = false;
    bool Up = false;
    bool Down = false;
    bool FastSpeed = false;
    bool Plus = false;
    bool Minus = false;
};


bool GLFWCameraHandler(CameraMovement& Movement, int Key, int Action, int Mods);
