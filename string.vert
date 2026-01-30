#version 330 core
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inCol;

out vec4 chCol;

uniform float amplitudes[6];
uniform float fretYNut[6];
uniform float fretYBridge[6];
uniform float time;
uniform float frequency;
uniform int vertsPerString;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    int lineIndex = (gl_VertexID / vertsPerString) % 6;

    float amp = amplitudes[lineIndex];

    float cutoffNut = fretYNut[lineIndex];
    float cutoffBridge = fretYBridge[lineIndex];

    float vibrateNut = (inPos.y < cutoffNut) ? 1.0 : 0.0;
    float vibrateBridge = (inPos.y > cutoffBridge) ? 1.0 : 0.0;
    float xOff = vibrateNut * vibrateBridge * amp * sin(time * frequency);

    vec3 pos = inPos + vec3(xOff, 0.0, 0.0);

    gl_Position = projection * view * model * vec4(pos, 1.0);
    chCol = vec4(inCol, 1.0);
}
