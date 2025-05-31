#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

//comes from vertex shader
in vec3 vPosition[];
in float vElevation[];
// goes to fragment shader 
out vec3 gNormal;    
out vec3 gPosition;  
out float elevation;

void main() {
    // Compute normal from triangle
    vec3 p0 = vPosition[0];
    vec3 p1 = vPosition[1];
    vec3 p2 = vPosition[2];

    vec3 normal = normalize(cross(p1 - p0, p2 - p0));

    for (int i = 0; i < 3; ++i) {
        gNormal = normal;
        elevation = vElevation[i];
        gPosition = vPosition[i];
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}