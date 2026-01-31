#pragma once

#include <vector>
#include <string>
#include <cstddef>
#include <cfloat>

#include <glm/glm.hpp>

#include <GL/glew.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct MeshData {
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> texcoords;
    std::vector<unsigned int> indices;
    GLuint diffuseTex = 0;
    glm::vec4 baseColorFactor = glm::vec4(1.0f);
};

struct GLMesh {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    GLuint texture = 0;
    glm::vec4 baseColorFactor;
    size_t indexCount = 0;
};

std::vector<MeshData> loadGLB(const std::string& path);
GLuint loadEmbeddedTexture(const aiScene* scene, const aiMaterial* mat);
GLMesh uploadMesh(const MeshData& md);
glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4& mat);
MeshData processMesh(aiMesh* mesh, const aiScene* scene, const glm::mat4& transform);
void processNode(aiNode* node, const aiScene* scene, std::vector<MeshData>& outMeshes, const glm::mat4& parentTransform);
void drawGuitarModel(unsigned int guitarModelShader,
    const std::vector<GLMesh>& guitarMeshes,
    glm::mat4 gProjection,
    glm::mat4 gView,
    glm::mat4 model,
    glm::vec3 gLightDir,
    glm::vec3 gLightColor,
    glm::vec3 cameraPos,
    glm::vec3 guitarCenter);