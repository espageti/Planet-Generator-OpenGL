#version 330 core

uniform vec3 v3LightPos;
uniform float g;
uniform float g2;

in vec3 v3Direction;
in vec4 primaryColor;
in vec4 secondaryColor;

out vec4 FragColor;

void main() {
    float fCos = dot(normalize(v3LightPos), normalize(v3Direction));
    float fMiePhase = 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCos*fCos) / pow(1.0 + g2 - 2.0*g*fCos, 1.5);
    
    float fRayleighPhase = 1.5 * ((1.0 - 0) / (2.0 + 0)) * (1.0 + fCos*fCos) / pow(1.0 + 0 - 2.0*0*fCos, 1.5);
    FragColor = primaryColor * fRayleighPhase + fMiePhase * secondaryColor;
}