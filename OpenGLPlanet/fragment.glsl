#version 330 core
out vec4 FragColor;

in vec3 gNormal;
in vec3 gPosition;
in float gElevation;
in vec3 gUnitSpherePos;

uniform vec3 lightPos;  
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform float seed;
#include "biomeDefs.glsl"
// Fade function
vec3 fade(vec3 t) {
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

// Hash function to generate pseudo-random float in [-1, 1]
float rand(vec3 p) {
    return fract(sin(dot(p ,vec3(127.1, 311.7, 74.7)) + seed) * 43758.5453) * 2.0 - 1.0;
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

float GenerateNoise(vec3 pointOnUnitSphere, float frequency, float persistence, int octaves, float roughness, float scaling)
{
    float noise = 0.0;
    for (int i = 0; i < octaves; i++)
    {
        vec3 p = pointOnUnitSphere * frequency;
        noise += perlinNoise(p) * scaling;
        frequency *= roughness;
        scaling *= persistence;
    }
    return 0.5 + 0.5 * noise;
}

void main() {

    //Ambient
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    vec3 norm = normalize(gNormal);
    vec3 lightDir = normalize(lightPos - gPosition);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    //Specular
    float specularStrength = 0.6;
    vec3 viewDir = normalize(viewPos - gPosition);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 4.0);
    vec3 specular = specularStrength * spec * lightColor;
    
    vec3 phong = diffuse + ambient + specular;
    
    float latitude = abs(gUnitSpherePos.y);
    float temp = 1.0 - latitude; // 1=equator, 0=pole
    
    float humidity = GenerateNoise(gUnitSpherePos, 2.1, 0.4, 4, 2.5, 0.5) - abs(gUnitSpherePos.y)/2.4; 
    
    float maxElevation = 0.1;
    vec3 vBiomeColor = calculateFinalBiomeColor(temp, humidity, gElevation / maxElevation, gUnitSpherePos);

    
    FragColor = vec4(vBiomeColor * phong, 1) ;
}