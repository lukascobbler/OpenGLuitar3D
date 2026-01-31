#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <filesystem>
#include <cfloat>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "ModelUtil.h"
#include "Util.h"

glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4& mat)
{
    return glm::mat4(
        mat.a1, mat.b1, mat.c1, mat.d1,
        mat.a2, mat.b2, mat.c2, mat.d2,
        mat.a3, mat.b3, mat.c3, mat.d3,
        mat.a4, mat.b4, mat.c4, mat.d4
    );
}

GLuint loadEmbeddedTexture(const aiScene* scene, const aiMaterial* mat)
{
    if (mat->GetTextureCount(aiTextureType_DIFFUSE) == 0) return 0;

    aiString texPath;
    mat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath);

    std::string path = texPath.C_Str();
    if (path.empty()) return 0;
    if (path[0] != '*') return 0;

    int texIndex = std::stoi(path.substr(1));
    aiTexture* tex = scene->mTextures[texIndex];

    int width, height, channels;
    unsigned char* data = nullptr;

    if (tex->mHeight == 0) {
        data = stbi_load_from_memory(
            reinterpret_cast<unsigned char*>(tex->pcData),
            tex->mWidth,
            &width, &height, &channels, 0
        );
    }
    else {
        width = tex->mWidth;
        height = tex->mHeight;
        channels = 4;
        data = new unsigned char[width * height * 4];
        memcpy(data, tex->pcData, width * height * 4);
    }

    if (!data) throw std::runtime_error("Failed to load embedded texture");

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    GLenum format = (channels == 3) ? GL_RGB : GL_RGBA;
    GLenum internalFormat = (channels == 3) ? GL_SRGB8 : GL_SRGB8_ALPHA8;

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (tex->mHeight != 0) delete[] data;
    else stbi_image_free(data);

    return texID;
}

MeshData processMesh(aiMesh* mesh, const aiScene* scene, const glm::mat4& transform)
{
    MeshData md;
    size_t baseVertex = 0;

    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        glm::vec4 v(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f);
        v = transform * v;

        md.vertices.push_back(v.x);
        md.vertices.push_back(v.y);
        md.vertices.push_back(v.z);

        if (mesh->HasNormals()) {
            md.normals.push_back(mesh->mNormals[i].x);
            md.normals.push_back(mesh->mNormals[i].y);
            md.normals.push_back(mesh->mNormals[i].z);
        }
        else md.normals.insert(md.normals.end(), { 0.f,0.f,0.f });

        if (mesh->HasTextureCoords(0)) {
            md.texcoords.push_back(mesh->mTextureCoords[0][i].x);
            md.texcoords.push_back(mesh->mTextureCoords[0][i].y);
        }
        else md.texcoords.insert(md.texcoords.end(), { 0.f,0.f });
    }

    for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
        aiFace face = mesh->mFaces[f];
        for (unsigned int i = 0; i < face.mNumIndices; ++i)
            md.indices.push_back(face.mIndices[i] + baseVertex);
    }

    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
        md.diffuseTex = loadEmbeddedTexture(scene, mat);

        aiColor4D baseColor(1.f, 1.f, 1.f, 1.f);
        mat->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor);
        md.baseColorFactor = glm::vec4(baseColor.r, baseColor.g, baseColor.b, baseColor.a);
    }

    return md;
}

void processNode(aiNode* node, const aiScene* scene, std::vector<MeshData>& outMeshes, const glm::mat4& parentTransform)
{
    glm::mat4 nodeTransform = parentTransform * aiMatrix4x4ToGlm(node->mTransformation);

    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        MeshData md = processMesh(mesh, scene, nodeTransform);
        outMeshes.push_back(md);
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i)
        processNode(node->mChildren[i], scene, outMeshes, nodeTransform);
}

std::vector<MeshData> loadGLB(const std::string& path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices);

    if (!scene || !scene->HasMeshes()) {
        std::cerr << "Assimp error: " << importer.GetErrorString() << std::endl;
        throw std::runtime_error("Failed to load GLB: " + path);
    }

    std::vector<MeshData> meshes;
    processNode(scene->mRootNode, scene, meshes, glm::mat4(1.0f));
    return meshes;
}

GLMesh uploadMesh(const MeshData& md)
{
    GLMesh m{};
    m.indexCount = md.indices.size();
    m.texture = md.diffuseTex;
    m.baseColorFactor = md.baseColorFactor;

    std::vector<float> interleaved;
    for (size_t i = 0; i < md.vertices.size() / 3; i++) {
        interleaved.push_back(md.vertices[i * 3 + 0]);
        interleaved.push_back(md.vertices[i * 3 + 1]);
        interleaved.push_back(md.vertices[i * 3 + 2]);

        interleaved.push_back(md.normals[i * 3 + 0]);
        interleaved.push_back(md.normals[i * 3 + 1]);
        interleaved.push_back(md.normals[i * 3 + 2]);

        interleaved.push_back(md.texcoords[i * 2 + 0]);
        interleaved.push_back(md.texcoords[i * 2 + 1]);
    }

    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);
    glGenBuffers(1, &m.ebo);

    glBindVertexArray(m.vao);

    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, interleaved.size() * sizeof(float), interleaved.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, md.indices.size() * sizeof(unsigned int), md.indices.data(), GL_STATIC_DRAW);

    int stride = 8 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    return m;
}

void drawGuitarModel(unsigned int guitarModelShader,
    const std::vector<GLMesh>& guitarMeshes,
    glm::mat4 gProjection,
    glm::mat4 gView,
    glm::mat4 model,
    glm::vec3 gLightDir,
    glm::vec3 gLightColor,
    glm::vec3 cameraPos,
    glm::vec3 guitarCenter)
{
    glUseProgram(guitarModelShader);

    glm::mat4 MVP = gProjection * gView * model;
    glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(model)));

    setMat4(guitarModelShader, "MVP", MVP);
    setMat4(guitarModelShader, "Model", model);
    setMat3(guitarModelShader, "NormalMatrix", normalMat);

    setVec3(guitarModelShader, "lightDir", gLightDir);
    setVec3(guitarModelShader, "lightColor", gLightColor);
    setVec3(guitarModelShader, "cameraPos", cameraPos);

    for (const auto& guitar : guitarMeshes)
    {
        glUniform4fv(glGetUniformLocation(guitarModelShader, "baseColorFactor"),
            1, glm::value_ptr(guitar.baseColorFactor));

        glActiveTexture(GL_TEXTURE0);
        if (guitar.texture != 0)
            glBindTexture(GL_TEXTURE_2D, guitar.texture);
        else
            glBindTexture(GL_TEXTURE_2D, 0);
        setInt(guitarModelShader, "tex", 0);

        glBindVertexArray(guitar.vao);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(guitar.indexCount), GL_UNSIGNED_INT, 0);
    }
}