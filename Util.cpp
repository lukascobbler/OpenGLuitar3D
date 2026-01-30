#include "Util.h";

#define _CRT_SECURE_NO_WARNINGS
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>

#include "stb_image.h"
#include "GuitarString.h"

unsigned int compileShader(GLenum type, const char* source)
{
    std::string content = "";
    std::ifstream file(source);
    std::stringstream ss;
    if (file.is_open())
    {
        ss << file.rdbuf();
        file.close();
        std::cout << "Successfully loaded textures \"" << source << "\"!" << std::endl;
    }
    else {
        ss << "";
        std::cout << "Texture loading failed \"" << source << "\"!" << std::endl;
    }
    std::string temp = ss.str();
    const char* sourceCode = temp.c_str(); 

    int shader = glCreateShader(type); 

    int success;
    char infoLog[512]; 
    glShaderSource(shader, 1, &sourceCode, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success); 
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        if (type == GL_VERTEX_SHADER)
            printf("VERTEX");
        else if (type == GL_FRAGMENT_SHADER)
            printf("FRAGMENT");
        printf(" shader error: \n");
        printf(infoLog);
    }
    return shader;
}

unsigned int createShader(const char* vsSource, const char* fsSource)
{
    //Pravi objedinjeni sejder program koji se sastoji od Vertex sejdera ciji je kod na putanji vsSource

    unsigned int program; //Objedinjeni sejder
    unsigned int vertexShader; //Verteks sejder (za prostorne podatke)
    unsigned int fragmentShader; //Fragment sejder (za boje, teksture itd)

    program = glCreateProgram(); //Napravi prazan objedinjeni sejder program

    vertexShader = compileShader(GL_VERTEX_SHADER, vsSource); //Napravi i kompajliraj vertex sejder
    fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource); //Napravi i kompajliraj fragment sejder

    //Zakaci verteks i fragment sejdere za objedinjeni program
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program); //Povezi ih u jedan objedinjeni sejder program
    glValidateProgram(program); //Izvrsi provjeru novopecenog programa

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success); //Slicno kao za sejdere
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(program, 512, NULL, infoLog);
        std::cout << "Objedinjeni sejder ima gresku! Greska: \n";
        std::cout << infoLog << std::endl;
    }

    //Posto su kodovi sejdera u objedinjenom sejderu, oni pojedinacni programi nam ne trebaju, pa ih brisemo zarad ustede na memoriji
    glDetachShader(program, vertexShader);
    glDeleteShader(vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(fragmentShader);

    return program;
}

unsigned loadImageToTexture(const char* filePath) {
    int width, height, channels;

    stbi_set_flip_vertically_on_load(1);

    unsigned char* data = stbi_load(filePath, &width, &height, &channels, 0);
    if (!data) {
        std::cout << "Texture not loaded: " << filePath << std::endl;
        return 0;
    }

    GLenum format = GL_RGB;
    GLenum internalFormat = GL_RGB;

    switch (channels) {
    case 1:
        format = internalFormat = GL_RED;
        break;
    case 3:
        format = GL_RGB;
        internalFormat = GL_SRGB;
        break;
    case 4:
        format = GL_RGBA;
        internalFormat = GL_SRGB_ALPHA;
        break;
    }

    unsigned texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);
    return texture;
}

GLFWcursor* loadImageToCursor(const char* filePath) {
    int TextureWidth;
    int TextureHeight;
    int TextureChannels;

    unsigned char* ImageData = stbi_load(filePath, &TextureWidth, &TextureHeight, &TextureChannels, 0);

    if (ImageData != NULL)
    {
        GLFWimage image;
        image.width = TextureWidth;
        image.height = TextureHeight;
        image.pixels = ImageData;

        int hotspotX = 2 * TextureWidth / 25;
        int hotspotY = 0;

        GLFWcursor* cursor = glfwCreateCursor(&image, hotspotX, hotspotY);
        stbi_image_free(ImageData);
        return cursor;
    } else {
        std::cout << "Cursor not loaded" << std::endl;
        stbi_image_free(ImageData);
    }
}

GLFWwindow* createFullScreenWindow(const char* windowName) {
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, windowName, monitor, NULL);

    return window;
}

void preprocessTexture(unsigned& texture, const char* filepath) {
    texture = loadImageToTexture(filepath);
    glBindTexture(GL_TEXTURE_2D, texture);

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

float cubicBezier(float t, float p0, float p1, float p2, float p3)
{
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    return uuu * p0 + 3.0f * uu * t * p1 + 3.0f * u * tt * p2 + ttt * p3;
}

void startCameraAnimation(CameraAnimation& anim,
    float newPitch, float newYaw, float newZoom,
    float duration,
    float startPitch, float startYaw, float startZoom)
{
    anim.isAnimating = true;
    anim.startTime = glfwGetTime();
    anim.duration = duration;

    anim.startPitch = startPitch;
    anim.startYaw = startYaw;
    anim.startZoom = startZoom;

    anim.targetPitch = newPitch;
    anim.targetYaw = newYaw;
    anim.targetZoom = newZoom;

    float pitchDelta = newPitch - startPitch;
    float yawDelta = newYaw - startYaw;
    float zoomDelta = newZoom - startZoom;

    anim.p1 = glm::vec2(
        startPitch + pitchDelta * 0.3f,
        startYaw + yawDelta * 0.3f
    );

    anim.p2 = glm::vec2(
        startPitch + pitchDelta * 0.7f,
        startYaw + yawDelta * 0.7f
    );

    anim.p1Zoom = startZoom + zoomDelta * 0.3f;
    anim.p2Zoom = startZoom + zoomDelta * 0.7f;
}

float pointLineDistance(float px, float py, float x0, float y0, float x1, float y1)
{
    // calculating the distance from a line using the normalized projection factor formula
    float dx = x1 - x0; float dy = y1 - y0;
    float lenSq = dx * dx + dy * dy;
    float t = ((px - x0) * dx + (py - y0) * dy) / lenSq;
    t = std::fmax(0.0f, std::fmin(1.0f, t));
    float projX = x0 + t * dx;
    float projY = y0 + t * dy;
    dx = px - projX;
    dy = py - projY;
    return std::sqrt(dx * dx + dy * dy);
}

void findClosestStringAndFret(float mouseXNDC, float mouseYNDC, const std::vector<GuitarString>& strings,
    int& outStringIndex, int& outFretIndex, float& outDistUp)
{
    outStringIndex = -1;
    outFretIndex = -1;
    outDistUp = -1;

    float minDist = 99999.0f;
    for (size_t i = 0; i < strings.size(); i++)
    {
        const auto& s = strings[i];
        float dist = pointLineDistance(mouseXNDC, mouseYNDC, s.x0, s.y0, s.x1, s.y1);
        if (dist < minDist)
        {
            minDist = dist;
            outStringIndex = (int)i;
        }
    }

    outDistUp = minDist;

    const auto& closestString = strings[outStringIndex];

    float minFretDist = 99999.0f;
    int closestFret = -1;

    for (size_t f = 0; f < closestString.fretMiddles.size(); f++)
    {
        float dx = mouseXNDC - closestString.fretMiddles[f][1];
        float dy = mouseYNDC - closestString.fretMiddles[f][2];
        float d = std::sqrt(dx * dx + dy * dy);
        if (d < minFretDist)
        {
            minFretDist = d;
            closestFret = (int)f;
        }
    }

    outFretIndex = closestFret;
}

glm::vec2 projectToScreen(glm::vec3 p,
    glm::mat4 model,
    glm::mat4 view,
    glm::mat4 proj,
    int width, int height)
{
    glm::vec4 clip = proj * view * model * glm::vec4(p, 1.0f);
    glm::vec3 ndc = glm::vec3(clip) / clip.w;

    glm::vec2 screen;
    screen.x = (ndc.x * 0.5f + 0.5f) * width;
    screen.y = (1.0f - (ndc.y * 0.5f + 0.5f)) * height;
    return screen;
}