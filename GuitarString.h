#pragma once

#include <GL/glew.h>
#include <vector>
#include <cmath>
#include <string>
#include <array>

struct GuitarString
{
    // display
    float x0, y0, z0;   // nut
    float x1, y1, z1;   // bridge
    float thickness;
    float r, g, b;
    std::string name;

    // vibration
    float vibrationTime = 0.0f;
    float currentAmplitude = 0.0f;
    bool isVibrating = false;
    bool hasBeenTriggered = false;

    // frets
    int fretPressed = 0;
    std::vector<std::array<float, 4>> fretMiddles;

    void computeFretMiddles();
};