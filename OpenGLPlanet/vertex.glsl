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

// Fade function
vec3 fade(vec3 t) {
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

// Hash function to generate pseudo-random float in [-1, 1]
float rand(vec3 p) {
    return fract(sin(dot(p ,vec3(127.1, 311.7, 74.7))) * 43758.5453) * 2.0 - 1.0;
}

// Generate a pseudo-random normalized 3D gradient vector
vec3 randomGradient(vec3 p) {
    float x = rand(p + vec3(1.0, 0.0, 0.0));
    float y = rand(p + vec3(0.0, 1.0, 0.0));
    float z = rand(p + vec3(0.0, 0.0, 1.0));
    return normalize(vec3(x, y, z));
}

// Perlin noise using random gradient vectors
float perlinNoise(vec3 pos) {
    vec3 i = floor(pos);
    vec3 pf = fract(pos);
    vec3 f = fade(pf);

    float n000 = dot(randomGradient(i + vec3(0,0,0)), pf - vec3(0,0,0));
    float n100 = dot(randomGradient(i + vec3(1,0,0)), pf - vec3(1,0,0));
    float n010 = dot(randomGradient(i + vec3(0,1,0)), pf - vec3(0,1,0));
    float n110 = dot(randomGradient(i + vec3(1,1,0)), pf - vec3(1,1,0));
    float n001 = dot(randomGradient(i + vec3(0,0,1)), pf - vec3(0,0,1));
    float n101 = dot(randomGradient(i + vec3(1,0,1)), pf - vec3(1,0,1));
    float n011 = dot(randomGradient(i + vec3(0,1,1)), pf - vec3(0,1,1));
    float n111 = dot(randomGradient(i + vec3(1,1,1)), pf - vec3(1,1,1));

    float x00 = mix(n000, n100, f.x);
    float x10 = mix(n010, n110, f.x);
    float x01 = mix(n001, n101, f.x);
    float x11 = mix(n011, n111, f.x);

    float y0 = mix(x00, x10, f.y);
    float y1 = mix(x01, x11, f.y);

    return mix(y0, y1, f.z);
}

// Evaluate layered noise on unit sphere
float EvaluateNoise(vec3 pointOnUnitSphere) {
    float elevation = 0.0;

    for (int i = 0; i < layerCount; i++) {
        if (!noiseLayers[i].enabled) continue;

        float frequency = noiseLayers[i].baseRoughness;
        float amplitude = 1.0;
        float layerValue = 0.0;

        for (int j = 0; j < noiseLayers[i].octaves; j++) {
            vec3 samplePoint = pointOnUnitSphere * frequency + noiseLayers[i].center;
            float noiseVal = perlinNoise(samplePoint);
            layerValue += noiseVal * amplitude;
            frequency *= noiseLayers[i].roughness;
            amplitude *= noiseLayers[i].persistence;
        }

        layerValue = layerValue * noiseLayers[i].strength - noiseLayers[i].minValue;
        elevation += max(0.0, 0.5 + 0.5 * layerValue);
    }

    return elevation;
}

// Estimate normal via spherical finite difference
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
    vec3 worldPos = unitSpherePos * radius * (1.0 + vElevation);

    vPosition = worldPos;
    gl_Position = projection * view * model * vec4(worldPos, 1.0);
    
}
