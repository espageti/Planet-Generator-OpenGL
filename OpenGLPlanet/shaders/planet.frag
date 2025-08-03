#version 330 core
out vec4 FragColor;

in vec3 gNormal;
in vec3 gPosition;
in float gElevation;
in vec3 gUnitSpherePos;

uniform vec3 v3LightPos;  
uniform vec3 v3LightColor;
uniform vec3 v3CameraPos;
uniform float maxElevation;
#include "biomeDefs.glsl"
#include "noise.glsl"

void main() {

    //Ambient
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * v3LightColor;

    // Diffuse
    vec3 norm = normalize(gNormal);
    vec3 lightDir = normalize(v3LightPos - gPosition);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * v3LightColor;

    //Specular
    float specularStrength = 0.6;
    vec3 viewDir = normalize(v3CameraPos - gPosition);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 4.0);
    vec3 specular = specularStrength * spec * v3LightColor;
    
    vec3 phong = diffuse + ambient + specular;
    
    float latitude = abs(gUnitSpherePos.y);
    float temp = 1.0 - latitude; // 1=equator, 0=pole
    
    float humidity = (GenerateNoise(gUnitSpherePos, 2.1, 0.4, 4, 2.5, 0.5) * 0.5 + 0.5) - abs(gUnitSpherePos.y)/2.4; 
    
    vec3 vBiomeColor = calculateFinalBiomeColor(temp, humidity, gElevation / maxElevation, gUnitSpherePos); // in biomeDefs.glsl

    
    FragColor = vec4(vBiomeColor * phong, 1) ;
}