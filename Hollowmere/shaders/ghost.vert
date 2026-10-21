#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec4 aColor;

out vec3 vWorldPos;
out vec3 vNormal;
out vec2 vUV;
out vec3 vViewDir;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;
uniform vec3 uCamPos;
uniform float uTime;

void main() {
    // Eerie ghost floating sway and breathing ripple
    vec3 pos = aPos;
    float wobble = sin(uTime * 2.8 + pos.y * 3.5) * 0.045;
    float drift = cos(uTime * 1.4 + pos.x * 2.0) * 0.03;
    pos.x += wobble;
    pos.z += drift;
    pos.y += sin(uTime * 1.8) * 0.06; // Floating levitation

    vec4 worldPos = uModel * vec4(pos, 1.0);
    vWorldPos = worldPos.xyz;
    vUV = aUV;

    mat3 normalMat = mat3(uModel);
    vNormal = normalize(normalMat * aNormal);
    vViewDir = normalize(uCamPos - worldPos.xyz);

    gl_Position = uProj * uView * worldPos;
}
