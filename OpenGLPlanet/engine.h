#pragma once

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include "mesh.h"
#include "noiseLayer.h"


void Init(GLFWwindow* window);
void RenderLoop(GLFWwindow* window);
void ProcessInput(GLFWwindow* window);
void Cleanup();
void SetNoiseLayers(const std::vector<NoiseLayer*> layers);
void MouseCallback(GLFWwindow* window, double xpos, double ypos);