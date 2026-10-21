#version 330 core

in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vUV;
in vec3 vViewDir;

out vec4 FragColor;

uniform float uTime;
uniform float uDissolve;    // 0.0 = fully visible, 1.0 = dissolved
uniform float uAgitation;   // 0.0 = calm/spectral cyan, 1.0 = enraged/blood red
uniform vec3 uLanternPos;
uniform vec3 uLanternDir;
uniform float uLanternCutoff;
uniform bool uLanternOn;

// Procedural noise for dissolve
float hash(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) + (c - a)* u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

void main() {
    // Procedural dissolve threshold
    float n = noise(vUV * 12.0 + vec2(uTime * 0.15));
    if (n < uDissolve) {
        discard;
    }

    // Fresnel rim effect (edges glow brightly)
    float fresnel = 1.0 - max(dot(vNormal, vViewDir), 0.0);
    fresnel = pow(fresnel, 2.8);

    // Lantern illumination calculation
    float lanternLight = 0.15; // Base ghostly luminescence
    if (uLanternOn) {
        vec3 toGhost = normalize(vWorldPos - uLanternPos);
        float theta = dot(toGhost, normalize(uLanternDir));
        if (theta > uLanternCutoff) {
            float dist = length(vWorldPos - uLanternPos);
            float atten = 1.0 / (1.0 + 0.1 * dist + 0.02 * dist * dist);
            lanternLight += (theta - uLanternCutoff) * atten * 1.8;
        }
    }

    // Spectral color palette: calm cyan/white vs enraged bloody crimson
    vec3 calmColor = vec3(0.55, 0.82, 0.95);
    vec3 enrageColor = vec3(0.95, 0.15, 0.12);
    vec3 ghostColor = mix(calmColor, enrageColor, uAgitation);

    // Rim glow and core translucency
    vec3 finalColor = ghostColor * (lanternLight + fresnel * 1.4);
    float alpha = clamp(0.35 + fresnel * 0.65 - uDissolve, 0.0, 1.0);

    FragColor = vec4(finalColor, alpha);
}
