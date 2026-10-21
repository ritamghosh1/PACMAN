#version 330 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uSceneTex;
uniform float uTime;
uniform float uFear;       // 0.0 to 1.0 (intensifies vignette, chromatic aberration)
uniform float uHealth;     // 1.0 (healthy) to 0.0 (critical, adds pulsing blood edges)
uniform float uSanity;     // 1.0 (sound) to 0.0 (distorted desaturation)

// High-speed pseudo-random for film grain
float random(vec2 p) {
    vec2 k1 = vec2(23.14069263277926, 2.665144142690225);
    return fract(cos(dot(p, k1)) * 12345.6789);
}

void main() {
    vec2 uv = vUV;

    // Chromatic aberration (spreads red and blue outwards when fear is high)
    float splitDist = (uFear * 0.012 + (1.0 - uHealth) * 0.008);
    vec2 dirFromCenter = uv - vec2(0.5);
    float rChannel = texture(uSceneTex, uv - dirFromCenter * splitDist).r;
    float gChannel = texture(uSceneTex, uv).g;
    float bChannel = texture(uSceneTex, uv + dirFromCenter * splitDist).b;

    vec3 sceneColor = vec3(rChannel, gChannel, bChannel);

    // Vignette
    float distFromCenter = length(dirFromCenter);
    float vignetteRadius = mix(0.75, 0.45, uFear);
    float vignetteSoftness = 0.45;
    float vignette = smoothstep(vignetteRadius, vignetteRadius - vignetteSoftness, distFromCenter);
    sceneColor *= vignette;

    // Blood edge vignette on low health
    if (uHealth < 0.6) {
        float pulse = (sin(uTime * 5.0) * 0.5 + 0.5);
        float bloodEdge = smoothstep(0.4, 0.75, distFromCenter);
        float bloodIntensity = (1.0 - (uHealth / 0.6)) * (0.6 + pulse * 0.4);
        vec3 bloodColor = vec3(0.5, 0.02, 0.02);
        sceneColor = mix(sceneColor, bloodColor, bloodEdge * bloodIntensity);
    }

    // Sanity desaturation
    if (uSanity < 0.8) {
        float gray = dot(sceneColor, vec3(0.299, 0.587, 0.114));
        float desat = (0.8 - uSanity) / 0.8;
        sceneColor = mix(sceneColor, vec3(gray), desat * 0.7);
    }

    // Film Grain
    float grain = (random(uv + vec2(sin(uTime), cos(uTime))) - 0.5) * (0.04 + uFear * 0.06);
    sceneColor += vec3(grain);

    FragColor = vec4(clamp(sceneColor, 0.0, 1.0), 1.0);
}
