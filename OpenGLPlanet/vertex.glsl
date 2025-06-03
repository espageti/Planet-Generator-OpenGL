#version 330 core

struct NoiseLayer {  
    bool enabled;
    float strength;
    float baseRoughness;
    float roughness;
    float persistence;
    int octaves;    
    float minValue;
    vec3 center;
};

layout (location = 0) in vec3 aPos;
uniform mat4 model, view, projection;
uniform float radius;
uniform int layerCount;
uniform NoiseLayer noiseLayers[8];

out vec3 vPosition;
out float vElevation;
out vec3 vUnitSpherePos;
#include "noise.glsl"

// Evaluate layered noise on unit sphere
float EvaluateNoise(vec3 pointOnUnitSphere) {
    float elevation = 0.0;

    for (int i = 0; i < layerCount; i++) {
        if (!noiseLayers[i].enabled) continue;

        float frequency = noiseLayers[i].baseRoughness;
        float layerValue = GenerateNoise(pointOnUnitSphere, frequency, noiseLayers[i].persistence, noiseLayers[i].octaves, noiseLayers[i].roughness, 1.0);

        layerValue = layerValue * noiseLayers[i].strength - noiseLayers[i].minValue;
        elevation += max(0.0, 0.5 + 0.5 * layerValue);
    }

    return elevation;
}

// Estimate normal via spherical finite difference (no longer using this)
vec3 ComputeNoiseNormal(vec3 unitSpherePos) {
    const float eps = 0.001;

    vec3 axisA = normalize(vec3(unitSpherePos.y, -unitSpherePos.x, 0.0));
    if (length(axisA) < 0.01) axisA = vec3(1.0, 0.0, 0.0);
    vec3 axisB = normalize(cross(unitSpherePos, axisA));

    float centerElevation = EvaluateNoise(unitSpherePos);
    vec3 center = unitSpherePos * (1.0 + centerElevation);

    vec3 offsetA = normalize(unitSpherePos + eps * axisA);
    vec3 offsetB = normalize(unitSpherePos + eps * axisB);

    vec3 pointA = offsetA * (1.0 + EvaluateNoise(offsetA));
    vec3 pointB = offsetB * (1.0 + EvaluateNoise(offsetB));

    vec3 tangent1 = pointA - center;
    vec3 tangent2 = pointB - center;

    return normalize(cross(tangent1, tangent2));
}

void main() {
    vec3 unitSpherePos = normalize(aPos);
    vElevation = EvaluateNoise(unitSpherePos);
    vec3 worldPos = (model * vec4(unitSpherePos * radius * (1.0 + vElevation), 1.0)).xyz;


    vPosition = worldPos;
    gl_Position = projection * view * vec4(worldPos, 1.0);
    
    vUnitSpherePos = unitSpherePos;
    
}
