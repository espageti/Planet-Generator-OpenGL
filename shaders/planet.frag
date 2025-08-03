#version 330 core
out vec4 FragColor;

in vec3 gNormal;
in vec3 gPosition;
in float gElevation;
in vec3 gUnitSpherePos;


uniform vec3 lightPos;  
uniform vec3 lightColor;
uniform vec3 cameraPos;
uniform float maxElevation;
uniform float exposure;

//atmosphere stuff
in vec3 gDirection;
in vec4 gRayleighColor;
in vec4 gMieColor;
uniform float gMie;
uniform float gMie2;

#include "biomeDefs.glsl"
#include "noise.glsl"

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
    vec3 viewDir = normalize(cameraPos - gPosition);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 4.0);
    vec3 specular = specularStrength * spec * lightColor;
    
    vec3 phong = diffuse + ambient + specular;
    
    float latitude = abs(gUnitSpherePos.y);
    float temp = 1.0 - latitude; // 1=equator, 0=pole
    
    float humidity = (GenerateNoise(gUnitSpherePos, 2.1, 0.4, 4, 2.5, 0.5) * 0.5 + 0.5) - abs(gUnitSpherePos.y)/2.4; 
    
    vec3 vBiomeColor = calculateFinalBiomeColor(temp, humidity, gElevation / maxElevation, gUnitSpherePos); // in biomeDefs.glsl
    float fCos = dot(normalize(lightPos), normalize(gDirection));

    float fMiePhase = 1.5 * ((1.0 - gMie2) / (2.0 + gMie2)) * (1.0 + fCos*fCos) / pow(1.0 + gMie2 - 2.0*gMie*fCos, 1.5);
    
    float fRayleighPhase = 0.75 * (1.0 + fCos*fCos);
    vec4 atmospheric = gRayleighColor * fRayleighPhase + fMiePhase * gMieColor;
    vec4 realColor = vec4(vBiomeColor * phong, 1);
    vec4 combinedColor = realColor * atmospheric * 0.6 + atmospheric * 0.1 + realColor * 0.4; // Add a bit of atmosphere to the final color
    FragColor = 1.0 -  exp(combinedColor * -exposure); // HDR (make bright a little less birght, dark a bit less dark)
}