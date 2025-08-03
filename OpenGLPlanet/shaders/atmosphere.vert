#version 330 core

layout (location = 0) in vec3 aPos;
uniform mat4 model, projection, view;

#include "common.vert"



void main(void) {
    vec3 v3Pos = (model * vec4(aPos, 1.0)).xyz;
    
    
    setScattering(v3Pos); // found in common.vert

    gl_Position = projection * view * vec4(v3Pos, 1.0);
}