#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

uniform mat4 MVP;
uniform mat4 Model;
uniform mat3 NormalMatrix;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoord;

void main()
{
    FragPos = vec3(Model * vec4(aPos, 1.0));

    Normal = normalize(NormalMatrix * aNormal);

    TexCoord = aUV;

    gl_Position = MVP * vec4(aPos, 1.0);
}
