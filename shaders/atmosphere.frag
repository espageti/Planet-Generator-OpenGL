#version 330 core

uniform vec3 lightPos;
uniform float gMie;
uniform float gMie2;
uniform float exposure;


in vec3 v3Direction;
in vec4 rayleighColor;
in vec4 mieColor;

out vec4 FragColor;

void main() {
    float fCos = dot(normalize(lightPos), normalize(v3Direction));
    float fMiePhase = 1.5 * ((1.0 - gMie2) / (2.0 + gMie2)) * (1.0 + fCos*fCos) / pow(1.0 + gMie2 - 2.0*gMie*fCos, 1.5);
    
    float fRayleighPhase = 0.75 * (1.0 + fCos*fCos);
    vec4 combinedColor = rayleighColor * fRayleighPhase + fMiePhase * mieColor;
    FragColor = 1.0 -  exp(combinedColor * -exposure);
}