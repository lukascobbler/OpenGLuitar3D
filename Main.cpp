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

// tests
bool isEnabledDepthTest = true;
bool isEnabledFaceCulling = true;

void toggleDepthTest()
{
    isEnabledDepthTest = !isEnabledDepthTest;
    if (isEnabledDepthTest) glEnable(GL_DEPTH_TEST);
    else glDisable(GL_DEPTH_TEST);
    std::cout << (isEnabledDepthTest ? "DEPTH TEST ENABLED" : "DEPTH TEST DISABLED") << std::endl;
}

void toggleFaceCulling()
{
    isEnabledFaceCulling = !isEnabledFaceCulling;
    if (isEnabledFaceCulling) glEnable(GL_CULL_FACE);
    else glDisable(GL_CULL_FACE);
    std::cout << (isEnabledFaceCulling ? "FACECULLING ENABLED" : "FACECULLING DISABLED") << std::endl;
}

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
        startCameraAnimation(animation, 89.9f, 89.9f, 0.5f, 0.3f, cameraPitch, cameraYaw, cameraDistance);
    }
    else {
        startCameraAnimation(animation, 89.9f, 89.9f, minDistance, 0.3f, cameraPitch, cameraYaw, cameraDistance);
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

    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        toggleDepthTest();
    }

    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        toggleFaceCulling();
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
    // GLFWwindow* window = glfwCreateWindow(1600, 900, "test", NULL, NULL);
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
        0.619167f, 0.94f,     0.0f, 1.0f,
        0.619167f, 0.731436f, 0.0f, 0.0f,
        0.94f,     0.731436f, 1.0f, 0.0f,
        0.94f,     0.94f,     1.0f, 1.0f
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

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, -guitarCenter);
    model = glm::rotate(model, glm::radians(-90.f), glm::vec3(1, 0, 0));
    model = glm::translate(model, guitarCenter);

    // light
    glm::vec3 gLightDir = guitarCenter + glm::vec3(0.0f, 5.0f, 0.0f);
    glm::vec3 gLightColor = glm::vec3(2.6f);

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

    formStringsVAO3D(strings, STRING_SEGMENTS, stringsVao, stringCount);

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
            startCameraAnimation(animation, 55.0f, 89.9f, minDistance, 1.5f, cameraPitch, cameraYaw, cameraDistance);
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
        drawGuitarModel(guitarModelShader, glMeshes, gProjection, gView, model, gLightDir, gLightColor, gCameraPos, guitarCenter);
        drawStrings3D(stringShader, stringsVao, stringCount, strings, gProjection, gView, model,
            lastHitFret, lastHitStringName, isPlayMode, isPressedLeft, isPressedRight, 
            screenMouseX, screenMouseY, width, height);

        if (isPlayMode) {
            drawFretCircles(circleShader, strings, gProjection, gView, model);
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