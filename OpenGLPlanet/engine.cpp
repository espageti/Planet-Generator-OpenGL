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
#include "shader.h"
#include "globals.h"
#include "planetUI.h"


unsigned int sphereVAO, sphereVBO, sphereEBO;
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
std::vector<float> sphereVertices;
std::vector<unsigned int> sphereIndices;

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

Shader* shader;
glm::mat4 projection;

glm::mat4 model;
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);


void Init(GLFWwindow* window) {
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

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
    shader = new Shader("shaders/vertex.glsl", "shaders/geometry.glsl", "shaders/fragment.glsl");
    shader->use();
    shader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);

    shape = new ShapeSettings(0.9f, 40);
    //Ocean layer
    NoiseLayer* ocean = new NoiseLayer();
    shape->AddNoiseLayer(ocean); 

    GenerateSphere(sphereVertices, sphereIndices);
    UploadMesh(sphereVAO, sphereVBO, sphereEBO, sphereVertices, sphereIndices);
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

        shader->use();

        // Set transformation matrices

        glm::mat4 view = glm::lookAt(
            cameraPos,
            cameraPos + cameraFront,
            cameraUp
        );
        model = glm::rotate(model, glm::radians(rotationSpeed), glm::vec3(0.0, 1.0, 0.0) );

        shader->setMat4("model", model);
        shader->setMat4("view", view);
        shader->setMat4("projection", projection);

        shader->setVec3("lightPos", lightPos);
        shader->setVec3("viewPos", cameraPos);
        // Draw mesh
        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);

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
    shader->use();

    for (int i = 0; i < layers.size() && i < 8; i++) {
        const NoiseLayer* layer = layers[i];
        std::string base = "noiseLayers[" + std::to_string(i) + "]";
        shader->setBool(base + ".enabled", layer->enabled);
        shader->setFloat(base + ".strength", layer->strength);
        shader->setInt(base + ".octaves", layer->octaves);
        shader->setFloat(base + ".baseRoughness", layer->baseRoughness);
        shader->setFloat(base + ".roughness", layer->roughness);
        shader->setFloat(base + ".persistence", layer->persistence);
        shader->setVec3(base + ".center", layer->center);
        shader->setFloat(base + ".minValue", layer->minValue);
    }
    shader->setFloat("radius", shape->radius);
    shader->setInt("layerCount", layers.size());
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
    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteBuffers(1, &sphereVBO);
    glDeleteBuffers(1, &sphereEBO);
    delete shader;
    delete shape;
    std::cout << "Cleanup done.\n";
}
