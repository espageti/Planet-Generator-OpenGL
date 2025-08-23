#include <glad/glad.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <filesystem>
//imgui
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "engine.h"

// FPS counter variables
double lastFrameTime = 0.0;
double currentFrameTime = 0.0;
double frameTime = 0.0;
double fps = 0.0;

double fpsUpdateInterval = 0.5; // Update FPS display every half second
double fpsUpdateTimer = 0.0;
int frameCount = 0;
double averageFps = 0.0;

glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, -10.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);  // Direction the camera is facing
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
//tangent to the sphere where the player is standing, should be identity when not in first person mode
glm::mat3 cameraBasis = glm::mat3(
    glm::vec3(1, 0, 0),  // right (x)
    glm::vec3(0, 1, 0),  // up    (y)
    glm::vec3(0, 0, 1)  // forward (z)
);

glm::mat4 rotation;

float yaw = 90.0f;  // Initialize facing towards Z
float pitch = 0.0f;
float lastX = 400.0f;  // Initial mouse position (center of screen)
float lastY = 300.0f;
bool firstMouse = true;

bool settingsMode = false;
bool firstPersonMode = false;

ShapeSettings* shape = nullptr;
float rotationSpeed = 0.0;
float densityFalloff = 1.0;
bool atmosphereEnabled = true;

Shader* planetShader;
Shader* atmosphereShader;
glm::mat4 projection;

glm::mat4 model;
glm::vec3 lightPos(0.0, 100.0f, -600.0f);

Sphere planet;
Sphere atmosphere;
float atmosphereThickness = 0.25;

float wavelengths[3];
float invWavelength4[3];

const float PI = 3.14159;
const int nSamples = 16;		// Number of sample rays to use in integral equation
const float kRayleigh = 0.0025f;		// Rayleigh scattering constant

const float kMie = 0.0010f;		// Mie scattering constant
float gMie = -0.99;  // Mie phase function asymmetry parameter
const float sunBrightness = 80.0f;		
const float exposure = 2.0f;


const float scatterStrength = 20;
void Init(GLFWwindow* window) {
    // Initialize last frame time
    lastFrameTime = glfwGetTime();

    wavelengths[0] = 650;		// 650 nm for red
    wavelengths[1] = 570;		// 570 nm for green
    wavelengths[2] = 475;		// 475 nm for blue
	invWavelength4[0] = powf(400 / wavelengths[0], 4.0f) * scatterStrength; // 400 / wavelength raised to the 4th power
    invWavelength4[1] = powf(400/wavelengths[1], 4.0f) * scatterStrength;
    invWavelength4[2] = powf(400/wavelengths[2], 4.0f) * scatterStrength;
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LEQUAL);

    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set background color
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);


    // Load shader
    planetShader = new Shader("shaders/planet.vert", "shaders/planet.frag", "shaders/planet.geom" );
    planetShader->enable();
    planetShader->setVec3("lightColor", lightColor);
    planetShader->setFloat("maxElevation", atmosphereThickness);


    shape = new ShapeSettings(4.0f, 50);
    //Ocean layer
    NoiseLayer* ocean = new NoiseLayer();
    shape->AddNoiseLayer(ocean); 

    planet.Create(shape->radius, shape->resolution);


    atmosphereShader = new Shader("shaders/atmosphere.vert", "shaders/atmosphere.frag");

    atmosphere.Create(shape->radius * (1.0 + atmosphereThickness), shape->resolution);
    SetNoiseLayers(shape->noiseLayers);

    rotation = glm::mat4(1.0f);
}

void UpdateFPS() {
    // Get current time
    currentFrameTime = glfwGetTime();
    
    // Calculate time since last frame
    frameTime = currentFrameTime - lastFrameTime;
    
    // Update last frame time
    lastFrameTime = currentFrameTime;
    
    // Increment frame counter
    frameCount++;
    
    // Update the timer
    fpsUpdateTimer += frameTime;
    
    // Only update the displayed FPS every update interval
    if (fpsUpdateTimer >= fpsUpdateInterval) {
        // Calculate average FPS over the interval
        averageFps = frameCount / fpsUpdateTimer;
        
        // Reset counters
        frameCount = 0;
        fpsUpdateTimer = 0.0;
    }
}

void RenderFPSCounter() {
    // Set up ImGui window in the top-right corner with FPS display
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 150, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(140, 70), ImGuiCond_FirstUseEver);
    
    // Create a small overlay window with minimal decorations
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | 
                                    ImGuiWindowFlags_AlwaysAutoResize | 
                                    ImGuiWindowFlags_NoSavedSettings |
                                    ImGuiWindowFlags_NoFocusOnAppearing |
                                    ImGuiWindowFlags_NoNav;
    
    ImGui::Begin("FPS Counter", nullptr, window_flags);
    ImGui::Text("FPS: %.1f", averageFps);
    ImGui::Text("Frame Time: %.2f ms", frameTime * 1000.0);
    ImGui::End();
}

void RenderLoop(GLFWwindow* window) {
    while (!glfwWindowShouldClose(window)) {


        // Set viewport and projection matrix
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float aspect = static_cast<float>(width) / static_cast<float>(height);
        projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
        model = glm::mat4(1.0f);
        glViewport(0, 0, width, height);

        // Update FPS at the start of each frame
        UpdateFPS();

        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "OpenGL error: " << err << std::endl;
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        planetShader->enable();

        // Set transformation matrices

        rotation = glm::rotate(rotation, glm::radians(rotationSpeed), glm::vec3(0, 1, 0));
        if (firstPersonMode)
        {
			glm::mat4 cameraRotation = glm::rotate(glm::mat4(1.0f), glm::radians(rotationSpeed), glm::vec3(0, 1, 0));
            cameraPos = glm::vec3(cameraRotation * glm::vec4(cameraPos, 1.0));
			std::cout << "Camera Position: " << cameraPos.x << " " << cameraPos.y << " " << cameraPos.z << std::endl;
             // Ensure direction is unit length
            cameraUp = glm::normalize(cameraUp);

            cameraBasis[0] = glm::vec3(cameraRotation * glm::vec4(cameraBasis[0], 1.0));
            glm::vec3 up = normalize(cameraPos);
            glm::vec3 forward = glm::normalize(glm::cross(up, -cameraBasis[0]));
            glm::vec3 right = -normalize(cross(forward, up));
            cameraBasis[0] = right;
            cameraBasis[1] = up;
            cameraBasis[2] = forward;

            std::cout << forward.x << " " << forward.y << " " << forward.z << std::endl;
        }
        glm::vec3 front = cameraBasis * glm::normalize(cameraFront);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + front, cameraUp);

        model = rotation * model;
        planetShader->setMat4("model", model);
        planetShader->setMat4("view", view);
        planetShader->setMat4("projection", projection);

        planetShader->setVec3("lightPos", lightPos);

        planetShader->setInt("nSamples", nSamples);
        planetShader->setVec3("cameraPos", cameraPos);
        planetShader->setVec3("lightPos", lightPos);
        planetShader->setVec3("lightColor", lightColor);
        planetShader->setVec3("invWavelength4", invWavelength4[0], invWavelength4[1], invWavelength4[2]);
        float cameraHeight = glm::length(cameraPos - glm::vec3(0, 0, 0));
        planetShader->setFloat("cameraHeight", cameraHeight);
        planetShader->setFloat("cameraHeight2", cameraHeight * cameraHeight);
        float atmosphereRadius = shape->radius * (1.0 + atmosphereThickness);
        planetShader->setFloat("atmosphereRadius", atmosphereRadius);
        planetShader->setFloat("atmosphereRadius2", atmosphereRadius * atmosphereRadius);
        float planetRadius = shape->radius;
        planetShader->setFloat("planetRadius", planetRadius);
        planetShader->setFloat("planetRadius2", planetRadius * planetRadius);

        planetShader->setFloat("kRayleighSunBrightness", kRayleigh * sunBrightness);
        planetShader->setFloat("kMieSunBrightness", kMie * sunBrightness);
        float scale = 1 / (atmosphereRadius - planetRadius);
        planetShader->setFloat("scale", scale);
        planetShader->setFloat("scaleDepth", 0.25); // the average density is found 25% of the way from ground to atmosphere
        
        planetShader->setFloat("gMie", gMie);
        planetShader->setFloat("gMie2", gMie * gMie);

        planetShader->setFloat("densityFalloff", densityFalloff);

        planetShader->setFloat("exposure", exposure);
        // Draw mesh
        planet.Draw();

        planetShader->disable();

        if (atmosphereEnabled)
        {



            atmosphereShader->enable();

            atmosphereShader->setMat4("model", model);
            atmosphereShader->setMat4("view", view);
            atmosphereShader->setMat4("projection", projection);
			atmosphereShader->setInt("nSamples", nSamples);
            atmosphereShader->setVec3("cameraPos", cameraPos);
            atmosphereShader->setVec3("lightPos", lightPos);
			atmosphereShader->setVec3("lightColor", lightColor);
            atmosphereShader->setVec3("invWavelength4", invWavelength4[0], invWavelength4[1], invWavelength4[2]);
            float cameraHeight = glm::length(cameraPos - glm::vec3(0, 0, 0));
            atmosphereShader->setFloat("cameraHeight", cameraHeight);
            atmosphereShader->setFloat("cameraHeight2", cameraHeight * cameraHeight);
            float atmosphereRadius = shape->radius * (1.0 + atmosphereThickness);
            atmosphereShader->setFloat("atmosphereRadius", atmosphereRadius);
            atmosphereShader->setFloat("atmosphereRadius2", atmosphereRadius * atmosphereRadius);
            float planetRadius = shape->radius;
            atmosphereShader->setFloat("planetRadius", planetRadius);
            atmosphereShader->setFloat("planetRadius2", planetRadius * planetRadius);

            atmosphereShader->setFloat("kRayleighSunBrightness", kRayleigh * sunBrightness);
            atmosphereShader->setFloat("kMieSunBrightness", kMie * sunBrightness);
            float scale = 1 / (atmosphereRadius - planetRadius);
            atmosphereShader->setFloat("scale", scale);
			atmosphereShader->setFloat("scaleDepth", 0.25); // the average density is found 25% of the way from ground to atmosphere
			std::cout << "gMie: " << gMie << std::endl;
            atmosphereShader->setFloat("gMie", gMie);
            atmosphereShader->setFloat("gMie2", gMie * gMie);

            atmosphereShader->setFloat("densityFalloff", densityFalloff);

            atmosphereShader->setFloat("exposure", exposure);

            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            glFrontFace(GL_CW);
            glDepthMask(GL_FALSE);
            // 1. Draw back faces of the atmosphere sphere
            glCullFace(GL_FRONT); // Cull the front faces, so only back faces are drawn
            atmosphere.Draw();

            // 2. Draw front faces of the atmosphere sphere
            glCullFace(GL_BACK); // Cull the back faces, so only front faces are drawn
            atmosphere.Draw();
            glDepthMask(GL_TRUE);
            glFrontFace(GL_CCW);
            glDisable(GL_BLEND);

            atmosphereShader->disable();
        }
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Display the FPS counter
        RenderFPSCounter();

        PlanetUI::DrawMainControls(shape, [&]() {
            SetNoiseLayers(shape->noiseLayers);
        });

        ImGuiIO& io = ImGui::GetIO();

        ProcessInput(window);
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
        glfwPollEvents();

    }
}

void SetNoiseLayers(const std::vector<NoiseLayer*> layers) {
    planetShader->enable();

    planetShader->setFloat("seed", shape->seed);

    for (int i = 0; i < layers.size() && i < 8; i++) {
        const NoiseLayer* layer = layers[i];
        std::string base = "noiseLayers[" + std::to_string(i) + "]";
        planetShader->setBool(base + ".enabled", layer->enabled);
        planetShader->setFloat(base + ".strength", layer->strength);
        planetShader->setInt(base + ".octaves", layer->octaves);
        planetShader->setFloat(base + ".baseRoughness", layer->baseRoughness);
        planetShader->setFloat(base + ".roughness", layer->roughness);
        planetShader->setFloat(base + ".persistence", layer->persistence);
        planetShader->setVec3(base + ".center", layer->center);
        planetShader->setFloat(base + ".minValue", layer->minValue);
    }
    planetShader->setInt("layerCount", layers.size());
    planetShader->disable();
}

void ProcessInput(GLFWwindow* window) {
    ImGuiIO& io = ImGui::GetIO();
    if (settingsMode && (io.WantCaptureMouse || io.WantCaptureKeyboard)) {
        return;
    }
    static bool escPressedLastFrame = false;
    bool escPressedThisFrame = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;

    // Toggle settings mode on key press (but only once per press)
    if (escPressedThisFrame && !escPressedLastFrame) {
        settingsMode = !settingsMode;

        if (settingsMode) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true;  // Reset mouse state
        }
    }
    escPressedLastFrame = escPressedThisFrame;

    if (!settingsMode) {

        glm::vec3 front = cameraBasis * cameraFront;
        float cameraSpeed = 2.5f * 0.016f;
        if (firstPersonMode)
        {

            glm::vec3 up = normalize(cameraPos);
            glm::vec3 right = normalize(cross(front, up));

            float velocity = cameraSpeed;

            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                cameraPos += front * velocity;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                cameraPos -= front * velocity;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                cameraPos -= right * velocity;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                cameraPos += right * velocity;

            // Clamp to surface
            cameraPos = normalize(cameraPos) * shape->radius;
            cameraPos *= 1.05;

            cameraUp = up;
        }
        else
        {
            float cameraSpeed = 2.5f * 0.016f;
            if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            {
                cameraSpeed *= 4;
            }
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                cameraPos += cameraSpeed * front;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                cameraPos -= cameraSpeed * front;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                cameraPos -= glm::normalize(glm::cross(front, cameraUp)) * cameraSpeed;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                cameraPos += glm::normalize(glm::cross(front, cameraUp)) * cameraSpeed;
        }  
    }
}



void MouseCallback(GLFWwindow* window, double xpos, double ypos) {
    // Forward mouse position to ImGui
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

    if (settingsMode) return;

    if (firstMouse) {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos);  // Reversed: y-coordinates range bottom to top
    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Clamp pitch
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    // Update camera front vector
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront =  glm::normalize(direction);
    cameraUp = cameraBasis * glm::vec3(0, 1, 0);
}

void Cleanup() {
    planet.Destroy();
    delete planetShader;
    delete shape;
    std::cout << "Cleanup done.\n";
}
