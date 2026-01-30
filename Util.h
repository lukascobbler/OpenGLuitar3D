#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h> 
#include <vector>
#include "GuitarString.h"
#include <glm/glm.hpp>

struct CameraAnimation {
    bool isAnimating = false;
    float time = 0.0f;
    double startTime;
    float duration = 2.0f;

    float startPitch, startYaw;
    float targetPitch, targetYaw;
    float startZoom, targetZoom;

    float p1Zoom, p2Zoom;
    glm::vec2 p1, p2;
};

unsigned int createShader(const char* vsSource, const char* fsSource);
unsigned loadImageToTexture(const char* filePath);
void preprocessTexture(unsigned& texture, const char* filepath);
GLFWcursor* loadImageToCursor(const char* filePath);
GLFWwindow* createFullScreenWindow(const char* windowName);

float cubicBezier(float t, float p0, float p1, float p2, float p3);
void startCameraAnimation(CameraAnimation& anim, float newPitch, float newYaw, float newZoom, 
    float duration, float startPitch, float startYaw, float startZoom);

float pointLineDistance(float px, float py, float x0, float y0, float x1, float y1);
void findClosestStringAndFret(float mouseXNDC, float mouseYNDC, const std::vector<GuitarString>& strings,
    int& outStringIndex, int& outFretIndex, float& outDistUp);
glm::vec2 projectToScreen(glm::vec3 p,
    glm::mat4 model,
    glm::mat4 view,
    glm::mat4 proj,
    int width, int height);