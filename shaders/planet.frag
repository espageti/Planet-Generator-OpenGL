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

//atmosphere stuff
in vec3 gDirection;
in vec4 gRayleighColor;
in vec4 gMieColor;
uniform float g;
uniform float g2;

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
    float fCos = dot(normalize(v3LightPos), normalize(gDirection));

    float fMiePhase = 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCos*fCos) / pow(1.0 + g2 - 2.0*g*fCos, 1.5);
    
    float fRayleighPhase = 0.75 * (1.0 + fCos*fCos);
    vec4 atmospheric = gRayleighColor * fRayleighPhase + fMiePhase * gMieColor;
    vec4 realColor = vec4(vBiomeColor * phong, 1);
    FragColor = realColor * atmospheric * 0.6 + atmospheric * 0.1 + realColor * 0.4; // Add a bit of atmosphere to the final color
}