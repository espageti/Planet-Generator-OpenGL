#version 330 core

uniform vec3 v3LightPos;
uniform float g;
uniform float g2;

in vec3 v3Direction;
in vec4 rayleighColor;
in vec4 mieColor;

out vec4 FragColor;

void main() {
    float fCos = dot(normalize(v3LightPos), normalize(v3Direction));
    float fMiePhase = 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCos*fCos) / pow(1.0 + g2 - 2.0*g*fCos, 1.5);
    
    float fRayleighPhase = 0.75 * (1.0 + fCos*fCos);
    FragColor = rayleighColor * fRayliehgPhase + fMiePhase * mieColor;
}