#version 330 core

in vec3 vFragPos;
in vec3 vNormal;
in vec2 vUV;
in mat3 vTBN;
in vec4 vColor;

out vec4 FragColor;

uniform sampler2D uDiffuseMap;
uniform sampler2D uNormalMap;
uniform bool uUseNormalMap;

// View pos
uniform vec3 uViewPos;

// Ambient
uniform vec3 uAmbientColor;

// Lantern Spotlight
struct SpotLightData {
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    float innerCutoff;
    float outerCutoff;
    float range;
    bool enabled;
};
uniform SpotLightData uSpotLight;

// Point Lights
struct PointLightData {
    vec3 position;
    vec3 color;
    float intensity;
    float range;
    bool enabled;
};
#define MAX_POINT_LIGHTS 4
uniform PointLightData uPointLights[MAX_POINT_LIGHTS];
uniform int uNumPointLights;

// Fog
uniform vec3 uFogColor;
uniform float uFogDensity;

void main() {
    vec4 texColor = texture(uDiffuseMap, vUV) * vColor;
    if (texColor.a < 0.1) discard;

    vec3 norm;
    if (uUseNormalMap) {
        vec3 normalSample = texture(uNormalMap, vUV).rgb;
        normalSample = normalSample * 2.0 - 1.0;
        norm = normalize(vTBN * normalSample);
    } else {
        norm = normalize(vNormal);
    }

    vec3 viewDir = normalize(uViewPos - vFragPos);

    // Ambient
    vec3 result = uAmbientColor * texColor.rgb;

    // Spot Light (Held Lantern)
    if (uSpotLight.enabled) {
        vec3 lightDir = normalize(uSpotLight.position - vFragPos);
        float dist = length(uSpotLight.position - vFragPos);

        // Distance attenuation
        float atten = clamp(1.0 - (dist / uSpotLight.range), 0.0, 1.0);
        atten = atten * atten;

        // Spot cone
        float theta = dot(lightDir, normalize(-uSpotLight.direction));
        float epsilon = uSpotLight.innerCutoff - uSpotLight.outerCutoff;
        float spotIntensity = clamp((theta - uSpotLight.outerCutoff) / epsilon, 0.0, 1.0);

        // Diffuse
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = uSpotLight.color * diff * texColor.rgb;

        // Specular (Blinn-Phong)
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(norm, halfwayDir), 0.0), 16.0);
        vec3 specular = uSpotLight.color * spec * 0.25;

        result += (diffuse + specular) * atten * spotIntensity * uSpotLight.intensity;
    }

    // Point Lights
    for (int i = 0; i < uNumPointLights && i < MAX_POINT_LIGHTS; ++i) {
        if (!uPointLights[i].enabled) continue;

        vec3 pLightDir = normalize(uPointLights[i].position - vFragPos);
        float pDist = length(uPointLights[i].position - vFragPos);

        float pAtten = clamp(1.0 - (pDist / uPointLights[i].range), 0.0, 1.0);
        pAtten = pAtten * pAtten;

        float pDiff = max(dot(norm, pLightDir), 0.0);
        vec3 pDiffuse = uPointLights[i].color * pDiff * texColor.rgb;

        vec3 pHalfway = normalize(pLightDir + viewDir);
        float pSpec = pow(max(dot(norm, pHalfway), 0.0), 12.0);
        vec3 pSpecular = uPointLights[i].color * pSpec * 0.2;

        result += (pDiffuse + pSpecular) * pAtten * uPointLights[i].intensity;
    }

    // Fog Calculation (exponential squared)
    float distToCam = length(uViewPos - vFragPos);
    float fogFactor = 1.0 - exp(-pow(distToCam * uFogDensity, 2.0));
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    vec3 finalColor = mix(result, uFogColor, fogFactor);

    FragColor = vec4(finalColor, texColor.a);
}
