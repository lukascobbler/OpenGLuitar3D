#include "Util.h";

#define _CRT_SECURE_NO_WARNINGS
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

void findClosestStringAndFret(
    float mouseX, float mouseY,
    const std::vector<GuitarString>& strings,
    const glm::mat4& model,
    const glm::mat4& view,
    const glm::mat4& proj,
    int width, int height,
    int& outStringIndex,
    int& outFretIndex,
    float& outDistUp)
{
    outStringIndex = -1;
    outFretIndex = -1;
    outDistUp = 999999.0f;

    float minDist = 999999.0f;

    for (int i = 0; i < strings.size(); i++)
    {
        const auto& s = strings[i];

        glm::vec3 p0(s.x0, s.y0, s.z0);
        glm::vec3 p1(s.x1, s.y1, s.z1);

        glm::vec2 s0 = projectToScreen(p0, model, view, proj, width, height);
        glm::vec2 s1 = projectToScreen(p1, model, view, proj, width, height);

        float dist = pointLineDistance(mouseX, mouseY,
            s0.x, s0.y, s1.x, s1.y);

        if (dist < minDist)
        {
            minDist = dist;
            outStringIndex = i;
        }
    }

    if (outStringIndex < 0) return;

    outDistUp = minDist;

    const auto& s = strings[outStringIndex];

    float minFretDist = 999999.0f;

    for (int f = 0; f < s.fretMiddles.size(); f++)
    {
        glm::vec3 p(
            s.fretMiddles[f][1],
            s.fretMiddles[f][2],
            s.fretMiddles[f][3]
        );

        glm::vec2 sp = projectToScreen(p, model, view, proj, width, height);

        float dx = mouseX - sp.x;
        float dy = mouseY - sp.y;
        float d = sqrt(dx * dx + dy * dy);

        if (d < minFretDist)
        {
            minFretDist = d;
            outFretIndex = f;
        }
    }
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

void drawCircle(unsigned int circleShader,
    const glm::vec3& center,
    const glm::mat4& gProjection,
    const glm::mat4& gView,
    const glm::mat4& model,
    float radius,
    int segments)
{
    std::vector<float> vertices;
    vertices.reserve((segments + 2) * 3);

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