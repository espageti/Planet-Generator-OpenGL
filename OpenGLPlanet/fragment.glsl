#version 330 core
out vec4 FragColor;

in vec3 gNormal;
in vec3 gPosition;
in float elevation;

uniform vec3 lightPos;  
uniform vec3 lightColor;
uniform vec3 viewPos;


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
    float specularStrength = 0.2;
    vec3 viewDir = normalize(viewPos - gPosition);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 4.0);
    vec3 specular = specularStrength * spec * lightColor;
    
    vec3 result = diffuse + ambient + specular;
    
    //biome coloring
    vec3 ocean = vec3(0.11, 0.20, 0.55);
    vec3 beach = vec3(0.84, 0.61, 0.11);
    vec3 grass = vec3(0.06, 0.56, 0.12);
    vec3 mountain = vec3(0.56, 0.28, 0.06);

    vec3 objectColor;
    float scaledElevation = elevation / 0.1;

    if (scaledElevation < 0.1) {
        objectColor = ocean;
    }
    else if (scaledElevation < 0.2) {
        float t = smoothstep(0.1, 0.2, scaledElevation);
        objectColor = mix(ocean, beach, t);
    }
    else if (scaledElevation < 0.5) {
        float t = smoothstep(0.2, 0.5, scaledElevation);
        objectColor = mix(beach, grass, t);
    }
    else {
        float t = smoothstep(0.4, 1.0, scaledElevation); 
        objectColor = mix(grass, mountain, t);
    }

    FragColor = vec4(objectColor * result, 1) ;
}