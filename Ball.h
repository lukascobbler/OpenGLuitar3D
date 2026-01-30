#pragma once

#include <GL/glew.h>

struct SphereMesh {
    GLuint vao = 0, vbo = 0, ebo = 0;
    size_t indexCount = 0;
};

SphereMesh createSphere(float radius, int sectors, int stacks);