#version 330 core

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

uniform sampler2D tex;
uniform vec3 cameraPos;
uniform vec4 baseColorFactor;

void main()
{
    vec3 N = normalize(Normal);

    vec3 texColor = texture(tex, TexCoord).rgb;
    vec3 albedo = (texColor.r == 0.0 && texColor.g == 0.0 && texColor.b == 0.0) 
             ? baseColorFactor.rgb 
             : texColor * baseColorFactor.rgb;

    vec3 ambient = 0.8 * albedo;

    vec3 V = normalize(cameraPos - FragPos);
    float rim = pow(1.0 - max(dot(N, V), 0.0), 2.0);
    vec3 rimLight = 0.1 * rim * albedo;

    vec3 finalColor = clamp(ambient + rimLight, 0.0, 1.0);

    FragColor = vec4(finalColor, 1.0);
}
