#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define NOMINMAX
#include <windows.h>
#include <thread>

#define _USE_MATH_DEFINES
#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>

#include "Util.h"
#include "GuitarString.h"
#include "Audio.h"
#include "ModelUtil.h"
#include "Ball.h"

// window
#define FPS 75
int width, height;
float aspectRatio;

// textures
unsigned signatureTexture;
unsigned int bgTexture;
unsigned int playmodeOffTexture;
unsigned int playmodeOnTexture;

// strings related stuff
std::vector<GuitarString> strings;
std::string lastHitStringName = "";
int lastHitFret = 0;
#define STRING_SEGMENTS 256
#define MAXIMUM_AMPLITUDE 0.002f
#define DECAY_RATE 1.0f
#define MARKER_RADIUS 0.004

// mouse + clicks
boolean isPressedLeft;
boolean isPressedRight;
GLFWcursor* cursorReleased;
GLFWcursor* cursorPressed;

// cursor and model position
double lastX = 0.0, lastY = 0.0;
bool rotatingCamera = false;
float modelYaw = 0.0f;
float modelPitch = 0.0f;
double screenMouseX = 0.0, screenMouseY = 0.0;

// camera and mouse scrolling
float cameraDistance = 2.0f;
float minDistance = 0.60f;
float maxDistance = 0.85f;
float cameraYaw = 0.0f;
float cameraPitch = 40.0f;
glm::vec3 gCameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
glm::vec3 guitarCenter;

// camera animation
CameraAnimation animation;
float deltaTime = 0;
boolean initialAnimation = true;

// keyboard related stuff
bool keyStates[1024] = { false };
bool isPlayMode = false;

// frame limiting
double lastTimeForRefresh;

// loading
std::atomic<bool> modelReadyCPU = false;

int endProgram(std::string message) {
    std::cout << message << std::endl;
    glfwTerminate();
    return -1;
}

void limitFPS()
{
    double now = glfwGetTime();
    double targetFrameTime = 1.0 / FPS;
    double remaining = (lastTimeForRefresh + targetFrameTime) - now;

    if (remaining > 0.0)
    {
        glfwWaitEventsTimeout(remaining);
    }
    else
    {
        glfwPollEvents();
    }

    lastTimeForRefresh = glfwGetTime();
}

void resetChord() {
    strings[0].fretPressed = 0; strings[1].fretPressed = 0; strings[2].fretPressed = 0;
    strings[3].fretPressed = 0; strings[4].fretPressed = 0; strings[5].fretPressed = 0;
}

void detectChords() {
    // major chords
    if (!keyStates[GLFW_KEY_LEFT_SHIFT]) {
        if (keyStates[GLFW_KEY_A]) {
            strings[0].fretPressed = -1; strings[1].fretPressed = 0; strings[2].fretPressed = 2;
            strings[3].fretPressed = 2; strings[4].fretPressed = 2; strings[5].fretPressed = 0;
        }
        else if (keyStates[GLFW_KEY_E]) {
            strings[0].fretPressed = 0; strings[1].fretPressed = 2; strings[2].fretPressed = 2;
            strings[3].fretPressed = 1; strings[4].fretPressed = 0; strings[5].fretPressed = 0;
        }
        else if (keyStates[GLFW_KEY_D]) {
            strings[0].fretPressed = -1; strings[1].fretPressed = -1; strings[2].fretPressed = 0;
            strings[3].fretPressed = 2; strings[4].fretPressed = 3; strings[5].fretPressed = 2;
        }
        else if (keyStates[GLFW_KEY_G]) {
            strings[0].fretPressed = 3; strings[1].fretPressed = 2; strings[2].fretPressed = 0;
            strings[3].fretPressed = 0; strings[4].fretPressed = 0; strings[5].fretPressed = 3;
        }
        else if (keyStates[GLFW_KEY_B]) {
            strings[0].fretPressed = -1; strings[1].fretPressed = 2; strings[2].fretPressed = 4;
            strings[3].fretPressed = 4; strings[4].fretPressed = 4; strings[5].fretPressed = 2;
        }
        else if (keyStates[GLFW_KEY_F]) {
            strings[0].fretPressed = 1; strings[1].fretPressed = 3; strings[2].fretPressed = 3;
            strings[3].fretPressed = 2; strings[4].fretPressed = 1; strings[5].fretPressed = 1;
        }
        else if (keyStates[GLFW_KEY_C]) {
            strings[0].fretPressed = -1; strings[1].fretPressed = 3; strings[2].fretPressed = 2;
            strings[3].fretPressed = 0; strings[4].fretPressed = 1; strings[5].fretPressed = 0;
        }
    }

    // minor chords
    else if (keyStates[GLFW_KEY_LEFT_SHIFT]) {
        if (keyStates[GLFW_KEY_A]) {
            strings[0].fretPressed = -1; strings[1].fretPressed = 0; strings[2].fretPressed = 2;
            strings[3].fretPressed = 2; strings[4].fretPressed = 1;   strings[5].fretPressed = 0;
        }
        else if (keyStates[GLFW_KEY_E]) {
            strings[0].fretPressed = 0; strings[1].fretPressed = 2; strings[2].fretPressed = 2;
            strings[3].fretPressed = 0; strings[4].fretPressed = 0; strings[5].fretPressed = 0;
        }
        else if (keyStates[GLFW_KEY_D]) {
            strings[0].fretPressed = -1; strings[1].fretPressed = -1; strings[2].fretPressed = 0;
            strings[3].fretPressed = 2; strings[4].fretPressed = 3; strings[5].fretPressed = 1;
        }
        else if (keyStates[GLFW_KEY_G]) {
            strings[0].fretPressed = 3; strings[1].fretPressed = 1; strings[2].fretPressed = 0;
            strings[3].fretPressed = 0; strings[4].fretPressed = 3; strings[5].fretPressed = 3;
        }
        else if (keyStates[GLFW_KEY_B]) {
            strings[0].fretPressed = -1; strings[1].fretPressed = 2; strings[2].fretPressed = 4;
            strings[3].fretPressed = 4; strings[4].fretPressed = 3; strings[5].fretPressed = 2;
        }
        else if (keyStates[GLFW_KEY_F]) {
            strings[0].fretPressed = 1; strings[1].fretPressed = 3; strings[2].fretPressed = 3;
            strings[3].fretPressed = 1; strings[4].fretPressed = 1; strings[5].fretPressed = 1;
        }
        else if (keyStates[GLFW_KEY_C]) {
            strings[0].fretPressed = -1; strings[1].fretPressed = 3; strings[2].fretPressed = 5;
            strings[3].fretPressed = 5; strings[4].fretPressed = 4; strings[5].fretPressed = 3;
        }
    }
}

void togglePlayMode() {
    if (animation.isAnimating) return;

    if (!isPlayMode) {
        startCameraAnimation(animation, 89.9, 89.9, 0.5, 0.3, cameraPitch, cameraYaw, cameraDistance);
    }
    else {
        startCameraAnimation(animation, 89.9, 89.9, minDistance, 0.3, cameraPitch, cameraYaw, cameraDistance);
    }

    isPlayMode = !isPlayMode;
}

void onetimeBtnPressCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) keyStates[key] = true;
        else if (action == GLFW_RELEASE) {
            resetChord();
            keyStates[key] = false;
        }
    }

    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        togglePlayMode();
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        endProgram("Program terminates!");
        exit(0);
    }
}

void mousePressCallback(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        glfwSetCursor(window, cursorPressed);
        isPressedLeft = button == GLFW_MOUSE_BUTTON_LEFT;
        isPressedRight = button == GLFW_MOUSE_BUTTON_RIGHT;

        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            rotatingCamera = true;
            glfwGetCursorPos(window, &lastX, &lastY);
        }
    }
    else if (action == GLFW_RELEASE) {
        glfwSetCursor(window, cursorReleased);

        isPressedLeft = false;
        isPressedRight = false;

        // right click reset
        lastHitFret = -1;
        lastHitStringName = "";

        // left click reset
        for (auto& string : strings) {
            string.hasBeenTriggered = false;
            string.fretPressed = 0;
        }

        rotatingCamera = false;
    }
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    screenMouseX = xpos;
    screenMouseY = ypos;

    if (isPlayMode) return;
    if (!rotatingCamera) return;

    float sensitivity = 0.6f;
    double dx = xpos - lastX;
    double dy = ypos - lastY;

    cameraYaw -= dx * sensitivity;
    cameraPitch += dy * sensitivity;

    if (cameraPitch > 89.9f) cameraPitch = 89.9f;
    if (cameraPitch < -89.9f) cameraPitch = -89.9f;

    // std::cout << "pitch: " << cameraPitch << ", yaw: " << cameraYaw << "\n";

    lastX = xpos;
    lastY = ypos;
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    // if (isPlayMode) return;

    float zoomSpeed = 0.01f;
    cameraDistance -= yoffset * zoomSpeed;

    if (cameraDistance < minDistance) cameraDistance = minDistance;
    if (cameraDistance > maxDistance) cameraDistance = maxDistance;
}

unsigned int createTexturedVAO(const float* vertices, size_t vertexCount) {
    unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };
    size_t indexCount = 6;

    unsigned int VAO, VBO, EBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(float), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    // layout: vec2 position, vec2 texcoord
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    return VAO;
}

void formStringsVAO3D(const std::vector<GuitarString>& strings, unsigned int& VAO, unsigned int& vertexCount)
{
    std::vector<float> vertices;

    const int radialSegments = 64;   // around the cylinder
    const int stringSegments = STRING_SEGMENTS; // along the string

    for (const auto& string : strings)
    {
        float radius = string.thickness * 0.5f;

        glm::vec3 start(string.x0, string.y0, string.z0);
        glm::vec3 end(string.x1, string.y1, string.z1);

        glm::vec3 axis = end - start;
        float length = glm::length(axis);
        if (length < 1e-6f) continue;

        glm::vec3 dir = glm::normalize(axis);

        // Find two vectors perpendicular to dir for the circular cross-section
        glm::vec3 up(0, 1, 0);
        if (fabs(glm::dot(dir, up)) > 0.99f) up = glm::vec3(1, 0, 0);
        glm::vec3 tangent = glm::normalize(glm::cross(dir, up));
        glm::vec3 bitangent = glm::normalize(glm::cross(dir, tangent));

        // Generate vertices
        for (int i = 0; i < stringSegments; i++)
        {
            float t0 = (float)i / stringSegments;
            float t1 = (float)(i + 1) / stringSegments;

            glm::vec3 p0 = start + dir * (length * t0);
            glm::vec3 p1 = start + dir * (length * t1);

            for (int j = 0; j < radialSegments; j++)
            {
                float theta0 = 2.0f * 3.14159265f * j / radialSegments;
                float theta1 = 2.0f * 3.14159265f * (j + 1) / radialSegments;

                glm::vec3 offset0 = tangent * cos(theta0) * radius + bitangent * sin(theta0) * radius;
                glm::vec3 offset1 = tangent * cos(theta1) * radius + bitangent * sin(theta1) * radius;

                glm::vec3 v0 = p0 + offset0;
                glm::vec3 v1 = p1 + offset0;
                glm::vec3 v2 = p1 + offset1;
                glm::vec3 v3 = p0 + offset1;

                float r = string.r;
                float g = string.g;
                float b = string.b;

                // First triangle
                vertices.insert(vertices.end(), { v0.x, v0.y, v0.z, r, g, b });
                vertices.insert(vertices.end(), { v1.x, v1.y, v1.z, r, g, b });
                vertices.insert(vertices.end(), { v2.x, v2.y, v2.z, r, g, b });

                // Second triangle
                vertices.insert(vertices.end(), { v0.x, v0.y, v0.z, r, g, b });
                vertices.insert(vertices.end(), { v2.x, v2.y, v2.z, r, g, b });
                vertices.insert(vertices.end(), { v3.x, v3.y, v3.z, r, g, b });
            }
        }
    }

    vertexCount = vertices.size() / 6; // 3 pos + 3 color

    unsigned int VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

// shader helpers
void setMat4(GLuint prog, const char* name, const glm::mat4& m) {
    glUniformMatrix4fv(glGetUniformLocation(prog, name),
        1, GL_FALSE, glm::value_ptr(m));
}
void setMat3(GLuint prog, const char* name, const glm::mat4& m) {
    glUniformMatrix3fv(glGetUniformLocation(prog, name),
        1, GL_FALSE, glm::value_ptr(m));
}
void setVec3(GLuint prog, const char* name, const glm::vec3& v) {
    glUniform3fv(glGetUniformLocation(prog, name), 1, glm::value_ptr(v));
}
void setVec4(GLuint prog, const char* name, const glm::vec4& v) {
    glUniform4fv(glGetUniformLocation(prog, name), 1, glm::value_ptr(v));
}
void setInt(GLuint prog, const char* name, int v) {
    glUniform1i(glGetUniformLocation(prog, name), v);
}
void setFloat(GLuint prog, const char* name, float v) {
    glUniform1f(glGetUniformLocation(prog, name), v);
}

void drawGuitarModel(unsigned int guitarModelShader,
    const std::vector<GLMesh>& guitarMeshes,
    glm::mat4 gProjection,
    glm::mat4 gView,
    glm::vec3 gLightDir,
    glm::vec3 gLightColor,
    glm::vec3 cameraPos,
    glm::vec3 guitarCenter)
{
    glUseProgram(guitarModelShader);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, -guitarCenter);
    model = glm::rotate(model, glm::radians(-90.f), glm::vec3(1, 0, 0));
    model = glm::translate(model, guitarCenter);

    glm::mat4 MVP = gProjection * gView * model;
    glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(model)));

    setMat4(guitarModelShader, "MVP", MVP);
    setMat4(guitarModelShader, "Model", model);
    setMat3(guitarModelShader, "NormalMatrix", normalMat);

    setVec3(guitarModelShader, "lightDir", gLightDir);
    setVec3(guitarModelShader, "lightColor", gLightColor);
    setVec3(guitarModelShader, "cameraPos", cameraPos);

    // Draw each mesh separately
    for (const auto& guitar : guitarMeshes)
    {
        // Base color factor
        glUniform4fv(glGetUniformLocation(guitarModelShader, "baseColorFactor"),
            1, glm::value_ptr(guitar.baseColorFactor));

        // Texture
        glActiveTexture(GL_TEXTURE0);
        if (guitar.texture != 0)
            glBindTexture(GL_TEXTURE_2D, guitar.texture);
        else
            glBindTexture(GL_TEXTURE_2D, 0);
        setInt(guitarModelShader, "tex", 0);

        // Draw
        glBindVertexArray(guitar.vao);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(guitar.indexCount), GL_UNSIGNED_INT, 0);
    }
}

void drawStrings3D(unsigned int stringShader,
    unsigned int stringsVAO,
    unsigned int stringsVertexCount,
    const glm::mat4& gProjection,
    const glm::mat4& gView,
    const glm::vec3& guitarCenter)
{
    float time = (float)glfwGetTime();

    std::vector<float> amplitudes;
    std::vector<float> fretYNut;
    std::vector<float> fretYBridge;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, -guitarCenter);
    model = glm::rotate(model, glm::radians(-90.f), glm::vec3(1, 0, 0));
    model = glm::translate(model, guitarCenter);

    for (auto& string : strings)
    {
        if (!isPlayMode) {
            continue;
        }

        bool trigger = false;

        glm::vec3 p0 = { string.x0, string.y0, string.z0 };
        glm::vec3 p1 = { string.x1, string.y1, string.z1 };

        glm::vec2 s0 = projectToScreen(p0, model, gView, gProjection, width, height);
        glm::vec2 s1 = projectToScreen(p1, model, gView, gProjection, width, height);

        glm::vec2 dir = glm::normalize(s1 - s0);
        glm::vec2 normal(-dir.y, dir.x);
        glm::vec3 p0_thick = p0 + glm::vec3(normal.x, normal.y, 0.0f) * string.thickness;
        glm::vec2 s0_thick = projectToScreen(p0_thick, model, gView, gProjection, width, height);

        float pixelThickness = glm::length(s0_thick - s0);

        // Left-click
        if (isPressedLeft) {
            float dist = pointLineDistance(screenMouseX, screenMouseY, s0.x, s0.y, s1.x, s1.y);
            if (dist < pixelThickness * 2.5f && string.fretPressed != -1) {
                trigger = !string.hasBeenTriggered;
                string.hasBeenTriggered = true;
            }
            else {
                string.hasBeenTriggered = false;
            }
        }

        // Right-click
        if (isPressedRight) {
            //int closestString, closestFret;
            //float distUp;
            //findClosestStringAndFret(screenMouseX, screenMouseY, strings, closestString, closestFret, distUp);

            //if (distUp < pixelThickness * 2.5f &&
            //    strings[closestString].name == string.name &&
            //    (closestFret != lastHitFret || string.name != lastHitStringName))
            //{
            //    string.fretPressed = closestFret;
            //    lastHitFret = closestFret;
            //    lastHitStringName = string.name;
            //    trigger = true;
            //    string.hasBeenTriggered = true;
            //}
        }

        if (trigger) {
            AudioEngine::playNote(string.name, string.fretPressed, 1);
            string.isVibrating = true;
            string.vibrationTime = 0.0f;
        }

        // Update vibration amplitude
        if (string.isVibrating) {
            string.vibrationTime += 0.016f;
            string.currentAmplitude = MAXIMUM_AMPLITUDE / (1.0f + DECAY_RATE * string.vibrationTime);
            if (string.currentAmplitude < 0.001f) {
                string.isVibrating = false;
                string.currentAmplitude = 0.0f;
            }
        }
        else {
            string.currentAmplitude = 0.0f;
        }

        // Determine fret cut
        float fretCut = string.y1;
        if (string.fretPressed >= 0 && string.fretPressed < string.fretMiddles.size()) {
            fretCut = string.fretMiddles[string.fretPressed][2];
        }

        fretYNut.push_back(fretCut);
        fretYBridge.push_back(string.y0 + 0.0099f);
        amplitudes.push_back(string.currentAmplitude);
    }

    glUseProgram(stringShader);

    glUniformMatrix4fv(glGetUniformLocation(stringShader, "model"), 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(stringShader, "projection"), 1, GL_FALSE, &gProjection[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(stringShader, "view"), 1, GL_FALSE, &gView[0][0]);

    glUniform1fv(glGetUniformLocation(stringShader, "amplitudes"), amplitudes.size(), amplitudes.data());
    glUniform1fv(glGetUniformLocation(stringShader, "fretYNut"), fretYNut.size(), fretYNut.data());
    glUniform1fv(glGetUniformLocation(stringShader, "fretYBridge"), fretYBridge.size(), fretYBridge.data());
    glUniform1f(glGetUniformLocation(stringShader, "time"), time);
    glUniform1f(glGetUniformLocation(stringShader, "frequency"), 200.0f);
    glUniform1i(glGetUniformLocation(stringShader, "vertsPerString"), STRING_SEGMENTS * 64 * 6); // radialSegments * triangles per segment
    glm::vec3 vibrationDir = glm::vec3(0, 0, 1);
    glUniform3fv(glGetUniformLocation(stringShader, "vibrationDir"), 1, &vibrationDir[0]);

    glBindVertexArray(stringsVAO);
    glDrawArrays(GL_TRIANGLES, 0, stringsVertexCount);
    glBindVertexArray(0);
}

void drawCircle3D(unsigned int circleShader,
    const glm::vec3& center,
    const glm::mat4& gProjection,
    const glm::mat4& gView,
    float radius,
    int segments = 32)
{
    std::vector<float> vertices;
    vertices.reserve((segments + 2) * 3);

    // center
    vertices.push_back(center.x);
    vertices.push_back(center.y);
    vertices.push_back(center.z);

    for (int i = 0; i <= segments; ++i)
    {
        float angle = i * 2.0f * glm::pi<float>() / segments;
        float x = center.x + radius * cos(angle);
        float y = center.y + radius * sin(angle);
        float z = center.z;

        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);
    }

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
        vertices.size() * sizeof(float),
        vertices.data(),
        GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        3 * sizeof(float), (void*)0);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, -guitarCenter);
    model = glm::rotate(model, glm::radians(-90.f), glm::vec3(1, 0, 0));
    model = glm::translate(model, guitarCenter);

    glUseProgram(circleShader);

    glUniformMatrix4fv(glGetUniformLocation(circleShader, "model"),
        1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(circleShader, "view"),
        1, GL_FALSE, &gView[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(circleShader, "projection"),
        1, GL_FALSE, &gProjection[0][0]);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, (GLsizei)(vertices.size() / 3));
    glBindVertexArray(0);

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

void drawFretCircles(unsigned int circleShader, const glm::mat4& gProjection, const glm::mat4& gView)
{
    for (auto& string : strings)
    {
        int fretPressed = string.fretPressed;
        if (fretPressed == -1) continue;

        auto& fretCenter = string.fretMiddles[fretPressed];

        glm::vec3 pos(
            fretCenter[1],
            fretCenter[2],
            fretCenter[3]
        );

        drawCircle3D(circleShader, pos, gProjection, gView, MARKER_RADIUS);
    }
}

void drawBall(unsigned int ballShader, const SphereMesh& mesh, glm::vec3 position,
    glm::mat4 gProjection,
    glm::mat4 gView)
{
    glUseProgram(ballShader);

    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    setMat4(ballShader, "Model", model);
    setMat4(ballShader, "View", gView);
    setMat4(ballShader, "Projection", gProjection);

    glBindVertexArray(mesh.vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.indexCount), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void drawTexture(unsigned int shader, unsigned int vao, unsigned int texture)
{
    glDepthMask(GL_FALSE);

    glUseProgram(shader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    setInt(shader, "bgTexture", 0);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glDepthMask(GL_TRUE);
}

void updateCameraAnimation()
{
    if (!animation.isAnimating) return;

    double now = glfwGetTime();
    float t = float((now - animation.startTime) / animation.duration);

    if (t >= 1.0f) {
        t = 1.0f;
        animation.isAnimating = false;
    }

    cameraPitch = cubicBezier(
        t,
        animation.startPitch,
        animation.p1.x,
        animation.p2.x,
        animation.targetPitch
    );

    cameraYaw = cubicBezier(
        t,
        animation.startYaw,
        animation.p1.y,
        animation.p2.y,
        animation.targetYaw
    );

    cameraDistance = cubicBezier(
        t,
        animation.startZoom,
        animation.p1Zoom,
        animation.p2Zoom,
        animation.targetZoom
    );
}

void moveCamera(glm::mat4& gView) {
    updateCameraAnimation();

    float radYaw = glm::radians(cameraYaw);
    float radPitch = glm::radians(cameraPitch);

    gCameraPos.x = guitarCenter.x + cameraDistance * cos(radPitch) * sin(radYaw);
    gCameraPos.y = guitarCenter.y + cameraDistance * sin(radPitch);
    gCameraPos.z = guitarCenter.z + cameraDistance * cos(radPitch) * cos(radYaw);

    gView = glm::lookAt(
        gCameraPos,
        guitarCenter,
        glm::vec3(0, 1, 0)
    );
}

void modelProgressBarAsync()
{
    int loadingStatus = 0;
    while (!modelReadyCPU) {
        loadingStatus = loadingStatus + 1;

        std::cout << "Loading status: " << loadingStatus << " / ~10000\n";
    }
}

int main()
{
    // glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // fullscreen window creation
    GLFWwindow* window = createFullScreenWindow("OpenGLuitar");
    // GLFWwindow* window = glfwCreateWindow(1280, 720, "test", NULL, NULL);
    if (window == NULL) return endProgram("Window did not create successfully.");
    glfwMakeContextCurrent(window);

    glfwGetFramebufferSize(window, &width, &height);
    aspectRatio = (float)width / height;

    // callback functions
    glfwSetKeyCallback(window, onetimeBtnPressCallback);
    glfwSetMouseButtonCallback(window, mousePressCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);

    // glew
    if (glewInit() != GLEW_OK) return endProgram("GLEW did not initialize successfully.");

    // alpha channel for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // audio
    AudioEngine::init();

    // cursor
    cursorReleased = loadImageToCursor("res/textures/cursor.png");
    cursorPressed = loadImageToCursor("res/textures/cursor_pressed.png");
    glfwSetCursor(window, cursorReleased);

    // textures
    preprocessTexture(signatureTexture, "res/textures/signature.png");
    preprocessTexture(bgTexture, "res/textures/background.png");
    preprocessTexture(playmodeOffTexture, "res/textures/playmode_off.png");
    preprocessTexture(playmodeOnTexture, "res/textures/playmode_on.png");

    // texture render definitions
    float bgVertices[] = {
        // pos      // tex
        -1.f, -1.f,  0.f, 0.f,
         1.f, -1.f,  1.f, 0.f,
         1.f,  1.f,  1.f, 1.f,
        -1.f,  1.f,  0.f, 1.f
    };
    unsigned int bgVAO = createTexturedVAO(bgVertices, 16);

    float verticesSignature[] = {
        0.619167f, 0.94, 0.0, 1.0,
        0.619167f, 0.731436, 0.0, 0.0,
        0.94f, 0.731436, 1.0, 0.0,
        0.94f, 0.94, 1.0, 1.0
    };

    float verticesPlaymode[] = {
        -0.9665f, -0.89f, 0.0f, 1.0f,
        -0.9665f, -0.94f, 0.0f, 0.0f,
        -0.7665f, -0.94f, 1.0f, 0.0f,
        -0.7665f, -0.89f, 1.0f, 1.0f
    };

    unsigned int sigVAO = createTexturedVAO(verticesSignature, 16);
    unsigned int playModeVao = createTexturedVAO(verticesPlaymode, 16);

    // shaders
    unsigned int guitarModelShader = createShader("guitar.vert", "guitar.frag");
    unsigned int textureShader = createShader("texture.vert", "texture.frag");
    unsigned int ballShader = createShader("ball.vert", "ball.frag");
    unsigned int stringShader = createShader("string.vert", "string.frag");
    unsigned int circleShader = createShader("circle.vert", "circle.frag");

    // loading
    std::thread loaderProgressBarThread(modelProgressBarAsync);
    loaderProgressBarThread.detach();

    // models
    std::vector<MeshData> meshes = loadGLB("res/models/acoustic_guitar_no_strings.glb");
    modelReadyCPU = true;

    std::cout << "Uploading to GPU...";

    std::vector<GLMesh> glMeshes;
    for (auto& md : meshes)
        glMeshes.push_back(uploadMesh(md));

    guitarCenter = glm::vec3(0.0, -0.09, -0.2);

    SphereMesh ballMesh = createSphere(0.01f, 64, 64);

    // camera
    glm::mat4 gView = glm::lookAt(
        gCameraPos,
        guitarCenter,
        glm::vec3(0, 1, 0)
    );

    glm::mat4 gProjection = glm::perspective(
        glm::radians(60.f),
        aspectRatio,
        0.1f,
        100.f
    );

    // light
    glm::vec3 gLightDir = guitarCenter + glm::vec3(0.0f, 5.0f, 0.0f);
    glm::vec3 gLightColor = glm::vec3(1.6f);

    // guitar strings

    strings = {
        { -0.02350f, 0.2000f, 0.0585f, -0.0170f, 0.830f, 0.044f, 0.0015f, 0.780f, 0.780f, 0.780f, "E"  },
        { -0.01400f, 0.2010f, 0.0585f, -0.0100f, 0.830f, 0.045f, 0.0015f, 0.780f, 0.780f, 0.780f, "A"  },
        { -0.00450f, 0.2015f, 0.0585f, -0.0030f, 0.830f, 0.045f, 0.0015f, 0.780f, 0.780f, 0.780f, "D"  },
        {  0.00475f, 0.2020f, 0.0585f,  0.0045f, 0.830f, 0.045f, 0.0013f, 1.000f, 1.000f, 1.000f, "G"  },
        {  0.01405f, 0.2030f, 0.0581f,  0.0115f, 0.830f, 0.045f, 0.0012f, 1.000f, 1.000f, 1.000f, "B"  },
        {  0.02370f, 0.2035f, 0.0580f,  0.0175f, 0.830f, 0.044f, 0.0012f, 1.000f, 1.000f, 1.000f, "Eh" }
    };

    unsigned int stringsVao;
    unsigned int stringCount;

    formStringsVAO3D(strings, stringsVao, stringCount);

    for (GuitarString& s : strings)
    {
        s.computeFretMiddles();
    }

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_FRAMEBUFFER_SRGB);

    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (initialAnimation) {
            startCameraAnimation(animation, 55.0, 89.9, minDistance, 1.5, cameraPitch, cameraYaw, cameraDistance);
            initialAnimation = false;
        }

        // texture draw
        drawTexture(textureShader, bgVAO, bgTexture);
        drawTexture(textureShader, sigVAO, signatureTexture);

        if (isPlayMode) {
            drawTexture(textureShader, playModeVao, playmodeOnTexture);
        }
        else {
            drawTexture(textureShader, playModeVao, playmodeOffTexture);
        }

        // camera update
        moveCamera(gView);

        // model render
        drawGuitarModel(guitarModelShader, glMeshes, gProjection, gView, gLightDir, gLightColor, gCameraPos, guitarCenter);
        drawStrings3D(stringShader, stringsVao, stringCount, gProjection, gView, guitarCenter);

        if (isPlayMode) {
            drawFretCircles(circleShader, gProjection, gView);
        }

        glfwPollEvents();

        detectChords();

        glfwSwapBuffers(window);

        AudioEngine::collectGarbage();
        limitFPS();
    }

    glfwDestroyWindow(window);
    AudioEngine::shutdown();
    glfwTerminate();
    return 0;
}

// sviranje desni klik
// ambijentalna muzika, 
// testovi za dubinu, 
// de-aifikovanje koda, 
// readme + kako instalirati,