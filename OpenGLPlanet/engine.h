#pragma once

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include "mesh.h"


void Init(GLFWwindow* window);
void RenderLoop(GLFWwindow* window);
void ProcessInput(GLFWwindow* window);
void Cleanup();
void MouseCallback(GLFWwindow* window, double xpos, double ypos);