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

unsigned int createTexturedVAO(const float* vertices, size_t vertexCount);

void drawTexture(unsigned int shader, unsigned int vao, unsigned int texture);
void drawCircle(unsigned int circleShader,
    const glm::vec3& center,
    const glm::mat4& gProjection,
    const glm::mat4& gView,
    const glm::mat4& model,
    float radius,
    int segments = 32);

float pointLineDistance(float px, float py, float x0, float y0, float x1, float y1);
void findClosestStringAndFret(
    float mouseX, float mouseY,
    const std::vector<GuitarString>& strings,
    const glm::mat4& model,
    const glm::mat4& view,
    const glm::mat4& proj,
    int width, int height,
    int& outStringIndex,
    int& outFretIndex,
    float& outDistUp);
glm::vec2 projectToScreen(glm::vec3 p,
    glm::mat4 model,
    glm::mat4 view,
    glm::mat4 proj,
    int width, int height);

void setMat4(GLuint prog, const char* name, const glm::mat4& m);
void setMat3(GLuint prog, const char* name, const glm::mat4& m);
void setVec3(GLuint prog, const char* name, const glm::vec3& v);
void setVec4(GLuint prog, const char* name, const glm::vec4& v);
void setInt(GLuint prog, const char* name, int v);
void setFloat(GLuint prog, const char* name, float v);