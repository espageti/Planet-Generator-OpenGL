# Procedural Planet Generator

A real time, GPU-accelerated procedural planet generator built with **OpenGL**, **GLSL**, and **C++**.

## Features
- Procedural sphere mesh generation with cube-to-sphere projection
- Configurable **multi-layered 3D noise** for terrain generation
- Real-time parameter editing and saving/loading with **ImGUI**
- Dynamic lighting, normal computation, and perlin noise calculation in **GLSL**
- Interactive camera

## Built With
- **C++**
- **OpenGL 3.3**
- **GLSL**
- **GLFW**
- **GLAD**
- **Dear ImGui**

## Controls
Look around with mouse, move around with WASD


## Build Instructions (Visual Studio 2022)
1. Clone the repo
2. Open .sln file in Visual Studio 2022
3. Link against glfw3, opengl32, and glad
4. Include directories for GLFW, GLAD, and Dear ImGUI
5. Build and run project
