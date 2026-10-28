#version 330 core

in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vUV;
in vec3 vViewDir;
in vec4 vColor;
in vec3 vLocalPos;

out vec4 FragColor;

uniform float uTime;
uniform float uDissolve;    // 0.0 = fully visible, 1.0 = dissolved
uniform float uAgitation;   // 0.0 = calm/spectral cyan, 1.0 = enraged/blood red
uniform vec3 uLanternPos;
uniform vec3 uLanternDir;
uniform float uLanternCutoff;
uniform bool uLanternOn;
uniform vec3 uTint;

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

    // Spectral colour: each ghost has its own cold tint; agitation drives it toward blood red
    vec3 enrageColor = vec3(0.95, 0.15, 0.12);
    vec3 ghostColor = mix(uTint, enrageColor, uAgitation);

    // Vertex colour carries the design: near-black parts are holes (eyes, mouths), saturated parts glow (ribbon, keys)
    float lum = dot(vColor.rgb, vec3(0.333));
    float sat = max(vColor.r, max(vColor.g, vColor.b)) - min(vColor.r, min(vColor.g, vColor.b));

    // Drifting mist: slow noise eats into the silhouette and makes the surface crawl
    float mist = noise(vLocalPos.xz * 7.0 + vec2(vLocalPos.y * 4.0 - uTime * 0.9, uTime * 0.4));
    float footFade = smoothstep(0.05, 0.75, vLocalPos.y);       // robes dissolve into vapour at the floor
    float breath = 0.85 + 0.15 * sin(uTime * 2.3);

    vec3 body = ghostColor * (0.30 + lanternLight + fresnel * 1.5) * breath * (0.25 + 0.75 * lum);
    vec3 finalColor = mix(body, vColor.rgb * (1.1 + lanternLight), clamp(sat * 1.6, 0.0, 0.85));
    finalColor += ghostColor * pow(fresnel, 1.5) * 0.4;

    float alpha = clamp(0.50 + fresnel * 0.5, 0.0, 1.0) * mix(0.18, 1.0, footFade) * (0.70 + 0.30 * mist);
    alpha = mix(alpha, 0.96, clamp(sat * 1.5, 0.0, 1.0));       // glowing details stay solid
    alpha *= 1.0 - 0.85 * smoothstep(0.7, 1.0, uDissolve);
    alpha = clamp(alpha - uDissolve * 0.35, 0.0, 1.0);

    FragColor = vec4(finalColor, alpha);
}
