#include <glad/glad.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <filesystem>
//imgui
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include "engine.h"



glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);  // Direction the camera is facing
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw = -90.0f;  // Initialize facing towards -Z
float pitch = 0.0f;
float lastX = 400.0f;  // Initial mouse position (center of screen)
float lastY = 300.0f;
bool firstMouse = true;

bool settingsMode = false;

ShapeSettings* shape = nullptr;
float rotationSpeed = 1.0;

Shader* planetShader;
Shader* atmosphereShader;
glm::mat4 projection;

glm::mat4 model;
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

Sphere planet;
Sphere atmosphere;

float m_fWavelength[3];
float m_fWavelength4[3];

const float PI = 3.14159;
const int m_nSamples = 3;		// Number of sample rays to use in integral equation
const float m_Kr = 0.0025f;		// Rayleigh scattering constant
const float m_Kr4PI = m_Kr * 4.0f * PI;
const float m_Km = 0.0010f;		// Mie scattering constant
const float m_Km4PI = m_Km * 4.0f * PI;
const float m_ESun = 20.0f;		// Sun brightness constant
const float m_g = -0.990f;		// The Mie phase asymmetry factor
const float m_fExposure = 2.0f;

void Init(GLFWwindow* window) {
    m_fWavelength[0] = 0.650f;		// 650 nm for red
    m_fWavelength[1] = 0.570f;		// 570 nm for green
    m_fWavelength[2] = 0.475f;		// 475 nm for blue
    m_fWavelength4[0] = powf(m_fWavelength[0], 4.0f);
    m_fWavelength4[1] = powf(m_fWavelength[1], 4.0f);
    m_fWavelength4[2] = powf(m_fWavelength[2], 4.0f);
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set background color
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    // Set viewport and projection matrix
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    float aspect = static_cast<float>(width) / static_cast<float>(height);
    projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    model = glm::mat4(1.0f);
    glViewport(0, 0, width, height);

    // Load shader
    planetShader = new Shader("shaders/planet.vert", "shaders/planet.frag", "shaders/planet.geom" );
    planetShader->enable();
    planetShader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);


    shape = new ShapeSettings(0.9f, 40);
    //Ocean layer
    NoiseLayer* ocean = new NoiseLayer();
    shape->AddNoiseLayer(ocean); 

    planet.Create(shape->radius, shape->resolution);


    atmosphereShader = new Shader("shaders/atmosphere.vert", "shaders/atmosphere.frag");

    atmosphere.Create(shape->radius * 1.5f, shape->resolution);
    SetNoiseLayers(shape->noiseLayers);
}

void RenderLoop(GLFWwindow* window) {
    while (!glfwWindowShouldClose(window)) {

        std::cout << "Working Directory: " << std::filesystem::current_path() << "\n";
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "OpenGL error: " << err << std::endl;
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        planetShader->enable();

        // Set transformation matrices

        glm::mat4 view = glm::lookAt(
            cameraPos,
            cameraPos + cameraFront,
            cameraUp
        );
        model = glm::rotate(model, glm::radians(rotationSpeed), glm::vec3(0.0, 1.0, 0.0) );

        planetShader->setMat4("model", model);
        planetShader->setMat4("view", view);
        planetShader->setMat4("projection", projection);

        planetShader->setVec3("lightPos", lightPos);
        planetShader->setVec3("viewPos", cameraPos);
        // Draw mesh
        planet.Draw();

        planetShader->disable();


        atmosphereShader->enable(); 
        
        atmosphereShader->setMat4("model", model);
        atmosphereShader->setMat4("view", view);
        atmosphereShader->setMat4("projection", projection);
        atmosphereShader->setVec3("v3CameraPos", cameraPos);
        atmosphereShader->setVec3("v3LightPos", lightPos/glm::length(lightPos));
        atmosphereShader->setVec3("v3InvWavelength", 1 / m_fWavelength4[0], 1 / m_fWavelength4[1], 1 / m_fWavelength4[2]);
        float cameraHeight = glm::length(cameraPos - glm::vec3(0, 0, 0)) - shape->radius;
        atmosphereShader->setFloat("fCameraHeight", cameraHeight);
        atmosphereShader->setFloat("fCameraHeight2", cameraHeight * cameraHeight);
        float atmosphereRadius = shape->radius * 1.5;
        atmosphereShader->setFloat("fOuterRadius", atmosphereRadius);
        atmosphereShader->setFloat("fOuterRadius2", atmosphereRadius * atmosphereRadius);
        float planetRadius = shape->radius;
        atmosphereShader->setFloat("fInnerRadius", planetRadius);
        atmosphereShader->setFloat("fInnerRadius2", planetRadius * planetRadius);

        atmosphereShader->setFloat("fKrESun", m_Kr * m_ESun);
        atmosphereShader->setFloat("fKmESun", m_Km * m_ESun);
        atmosphereShader->setFloat("fKr4PI", m_Kr4PI);
        atmosphereShader->setFloat("fKm4PI", m_Km4PI);
        float scale = 1 / (atmosphereRadius - planetRadius);
        atmosphereShader->setFloat("fScale", scale);
        atmosphereShader->setFloat("fScaleDepth", 0.25);
        atmosphereShader->setFloat("fScaleOverScaleDepth", scale / 0.25); 
        float m_g = -0.990f;		// The Mie phase asymmetry factor
        atmosphereShader->setFloat("g", m_g);
        atmosphereShader->setFloat("g2", m_g * m_g);


        glDepthMask(GL_FALSE);  
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE); 
        glDisable(GL_CULL_FACE);      
        atmosphere.Draw();
        glEnable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);

        atmosphereShader->disable();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

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
        const float cameraSpeed = 2.5f * 0.016f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
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
    cameraFront = glm::normalize(direction);
}


void Cleanup() {
    planet.Destroy();
    delete planetShader;
    delete shape;
    std::cout << "Cleanup done.\n";
}
