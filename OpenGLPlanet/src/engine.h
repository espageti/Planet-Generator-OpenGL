#pragma once

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include "noiseLayer.h"

#include "shader.h"
#include "globals.h"
#include "planetUI.h"
#include "sphere.h"

// FPS counter variables
extern double lastFrameTime;
extern double currentFrameTime;
extern double frameTime;
extern double fps;

void Init(GLFWwindow* window);
void RenderLoop(GLFWwindow* window);
void ProcessInput(GLFWwindow* window);
void Cleanup();
void SetNoiseLayers(const std::vector<NoiseLayer*> layers);
void MouseCallback(GLFWwindow* window, double xpos, double ypos);
void UpdateFPS();
void RenderFPSCounter();