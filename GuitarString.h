#pragma once

#include <GL/glew.h>
#include <vector>
#include <cmath>
#include <string>
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STRING_SEGMENTS 256
#define MAXIMUM_AMPLITUDE 0.002f
#define DECAY_RATE 1.0f

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

void drawStrings3D(unsigned int stringShader,
    unsigned int stringsVAO,
    unsigned int stringsVertexCount,
    std::vector<GuitarString>& strings,
    const glm::mat4& gProjection,
    const glm::mat4& gView,
    const glm::mat4& model,
    int& lastHitFret,
    std::string& lastHitStringName,
    bool isPlayMode,
    bool isPressedLeft,
    bool isPressedRight,
    float screenMouseX,
    float screenMouseY,
    int width,
    int height);

void formStringsVAO3D(const std::vector<GuitarString>& strings,
    const int stringSegments, unsigned int& VAO, unsigned int& vertexCount);

void drawFretCircles(unsigned int circleShader,
    const std::vector<GuitarString>& strings,
    const glm::mat4& gProjection,
    const glm::mat4& gView,
    const glm::mat4& model);