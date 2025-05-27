#include <glad/glad.h>
#include "engine.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "shader.h"
//imgui
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "globals.h"


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

Shader* shader;
glm::mat4 projection;
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
    glViewport(0, 0, width, height);

    // Load shader
    shader = new Shader("vertex.glsl", "fragment.glsl");
    shader->use();
    shader->setVec3("objectColor", 1.0f, 0.5f, 0.31f);
    shader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);

    shape = new ShapeSettings(0.9f, 40);
    //Ocean layer
    NoiseSettings* ocean = new NoiseSettings();
    shape->AddNoiseLayer(ocean); 

    GenerateSphere(sphereVertices, sphereIndices);
    UploadMesh(sphereVAO, sphereVBO, sphereEBO, sphereVertices, sphereIndices);
}

void RenderLoop(GLFWwindow* window) {
    while (!glfwWindowShouldClose(window)) {


        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader->use();

        // Set transformation matrices
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::lookAt(
            cameraPos,
            cameraPos + cameraFront,
            cameraUp
        );
        shader->setMat4("model", model);
        shader->setMat4("view", view);
        shader->setMat4("projection", projection);

        shader->setVec3("lightPos", lightPos);

        // Draw mesh
        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Debug UI
        ImGui::Begin("Planet Window");
        ImGui::SliderInt("Resolution", &shape->resolution, 2, 50);
        ImGui::SliderFloat("Planet Radius",  &shape->radius, 0.0f, 10.0f);
        if (ImGui::Button("Regenerate")) {
            
            std::cout << "Regenerating with radius: " << shape->radius << std::endl;
            sphereVertices.clear();
            sphereIndices.clear();
            GenerateSphere(sphereVertices, sphereIndices); 
            UploadMesh(sphereVAO, sphereVBO, sphereEBO, sphereVertices, sphereIndices);
            std::cout << "Vertices: " << sphereVertices.size() << ", Indices: " << sphereIndices.size() << std::endl;
        }
        int i = 0;
        for (NoiseSettings* layer : shape->noiseLayers) {
            ImGui::PushID(i); // Unique identifier for each layer

            ImGui::Separator();
            ImGui::Text("Layer %d", i);
            ImGui::Checkbox("Enabled", &layer->enabled);

            if (layer->enabled) {
                ImGui::SliderFloat("Strength", &layer->strength, 0.0f, 10.0f);
                ImGui::SliderFloat("Roughness", &layer->roughness, 0.0f, 5.0f);
                ImGui::SliderFloat("Base Roughness", &layer->baseRoughness, 0.0f, 5.0f);
                ImGui::SliderInt("Octaves", &layer->octaves, 1, 10);
                ImGui::SliderFloat("Persistence", &layer->persistence, 0.0f, 1.0f);
                ImGui::SliderFloat("Min Value", &layer->minValue, 0.0f, 2.0f);

            }

            ImGui::PopID();
            i++;
        }
        ImGui::End();

        ImGuiIO& io = ImGui::GetIO();

        ProcessInput(window);
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
        glfwPollEvents();

    }
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
