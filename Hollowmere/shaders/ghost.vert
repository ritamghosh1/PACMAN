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
out vec4 vColor;
out vec3 vLocalPos;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;
uniform vec3 uCamPos;
uniform float uTime;

void main() {
    // Eerie ghost floating sway and breathing ripple
    vec3 pos = aPos;
    float sway = smoothstep(0.2, 2.0, aPos.y);            // feet stay put, upper body drifts
    float wobble = sin(uTime * 2.2 + pos.y * 3.0) * 0.03 * sway;
    float drift = cos(uTime * 1.3 + pos.x * 2.0) * 0.02 * sway;
    pos.x += wobble;
    pos.z += drift;
    pos.y += 0.10 + sin(uTime * 1.6) * 0.05; // levitation: ghosts hover just above the boards

    vec4 worldPos = uModel * vec4(pos, 1.0);
    vWorldPos = worldPos.xyz;
    vUV = aUV;
    vColor = aColor;
    vLocalPos = aPos;

    mat3 normalMat = mat3(uModel);
    vNormal = normalize(normalMat * aNormal);
    vViewDir = normalize(uCamPos - worldPos.xyz);

    gl_Position = uProj * uView * worldPos;
}
