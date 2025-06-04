#pragma once

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include "noiseLayer.h"

#include "shader.h"
#include "globals.h"
#include "planetUI.h"
#include "sphere.h"


void Init(GLFWwindow* window);
void RenderLoop(GLFWwindow* window);
void ProcessInput(GLFWwindow* window);
void Cleanup();
void SetNoiseLayers(const std::vector<NoiseLayer*> layers);
void MouseCallback(GLFWwindow* window, double xpos, double ypos);