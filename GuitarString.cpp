#include "GuitarString.h"
#include "Util.h"
#include "Audio.h"
#include <glm/glm.hpp>

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
    int height)
{
    float time = (float)glfwGetTime();

    std::vector<float> amplitudes;
    std::vector<float> fretYNut;
    std::vector<float> fretYBridge;

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

        if (isPressedRight) {
            int closestString, closestFret;
            float distUp;
            findClosestStringAndFret(
                screenMouseX, screenMouseY,
                strings,
                model, gView, gProjection,
                width, height, closestString, closestFret, distUp);

            if (distUp < pixelThickness * 2.5f &&
                strings[closestString].name == string.name &&
                (closestFret != lastHitFret || string.name != lastHitStringName))
            {
                string.fretPressed = closestFret;
                lastHitFret = closestFret;
                lastHitStringName = string.name;
                trigger = true;
                string.hasBeenTriggered = true;
            }
        }

        if (trigger) {
            AudioEngine::playNote(string.name, string.fretPressed, 1);
            string.isVibrating = true;
            string.vibrationTime = 0.0f;
        }

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
    glUniform1i(glGetUniformLocation(stringShader, "vertsPerString"), STRING_SEGMENTS * 64 * 6);
    glm::vec3 vibrationDir = glm::vec3(0, 0, 1);
    glUniform3fv(glGetUniformLocation(stringShader, "vibrationDir"), 1, &vibrationDir[0]);

    glBindVertexArray(stringsVAO);
    glDrawArrays(GL_TRIANGLES, 0, stringsVertexCount);
    glBindVertexArray(0);
}

void GuitarString::computeFretMiddles()
{
    fretMiddles.clear();
    fretMiddles.reserve(21);

    glm::vec3 bridge(x0, y0, z0); // smaller y
    glm::vec3 nut(x1, y1, z1);    // bigger y

    fretMiddles.push_back({ -1.0f, nut.x, nut.y - 0.002f, nut.z + 0.0009f });

    glm::vec3 prevPos = nut;

    for (int n = 0; n <= 20; n++)
    {
        float f = 1.0f - 1.0f / std::pow(2.0f, n / 12.3f);

        glm::vec3 pos = nut + f * (bridge - nut);

        glm::vec3 mid = (prevPos + pos) * 0.5f;

        if (n > 0) fretMiddles.push_back({ (float)(n - 1), mid.x, mid.y - 0.0025f, mid.z + 0.0009f });

        prevPos = pos;
    }
}

void formStringsVAO3D(const std::vector<GuitarString>& strings, const int stringSegments, unsigned int& VAO, unsigned int& vertexCount)
{
    std::vector<float> vertices;

    const int radialSegments = 64;

    for (const auto& string : strings)
    {
        float radius = string.thickness * 0.5f;

        glm::vec3 start(string.x0, string.y0, string.z0);
        glm::vec3 end(string.x1, string.y1, string.z1);

        glm::vec3 axis = end - start;
        float length = glm::length(axis);
        if (length < 1e-6f) continue;

        glm::vec3 dir = glm::normalize(axis);

        glm::vec3 up(0, 1, 0);
        if (fabs(glm::dot(dir, up)) > 0.99f) up = glm::vec3(1, 0, 0);
        glm::vec3 tangent = glm::normalize(glm::cross(dir, up));
        glm::vec3 bitangent = glm::normalize(glm::cross(dir, tangent));

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

                vertices.insert(vertices.end(), { v0.x, v0.y, v0.z, r, g, b });
                vertices.insert(vertices.end(), { v1.x, v1.y, v1.z, r, g, b });
                vertices.insert(vertices.end(), { v2.x, v2.y, v2.z, r, g, b });

                vertices.insert(vertices.end(), { v0.x, v0.y, v0.z, r, g, b });
                vertices.insert(vertices.end(), { v2.x, v2.y, v2.z, r, g, b });
                vertices.insert(vertices.end(), { v3.x, v3.y, v3.z, r, g, b });
            }
        }
    }

    vertexCount = vertices.size() / 6;

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

void drawFretCircles(unsigned int circleShader,
    const std::vector<GuitarString>& strings,
    const glm::mat4& gProjection,
    const glm::mat4& gView,
    const glm::mat4& model)
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

        drawCircle(circleShader, pos, gProjection, gView, model, 0.004);
    }
}