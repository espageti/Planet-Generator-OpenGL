# Procedural Planet Generator

A real time, GPU-accelerated procedural planet generator built with **OpenGL**, **GLSL**, and **C++**.

## Features
- Procedural sphere mesh generation with cube-to-sphere projection
- Configurable **multi-layered 3D noise** for terrain generation
- Real-time parameter editing and saving/loading with **ImGUI**
- Dynamic lighting, normal computation, and perlin noise calculation in **GLSL**
- Interactive camera
- Atmospheric scattering

## Built With
- **C++**
- **OpenGL 3.3**
- **GLSL**
- **GLFW**
- **GLAD**
- **Dear ImGui**

## Controls
Look around with mouse, move around with WASD

## Prerequisites
CMake (version 3.15 or newer recommended)

A C++ compiler (e.g., MSVC on Windows, gcc or clang on Linux/macOS)

(Optional) vcpkg if you use it for dependencies

## Build Instructions
Open a terminal or command prompt, then run the following commands from the root directory of the project:

cmake -S . -B build

cmake --build build

## Author Contributions

This project was fully designed and implemented by me, Darren Lin.

I built the procedural mesh generation, noise layering system, GLSL shaders for terrain and atmosphere rendering, interactive camera, and the ImGui-based UI. All code in the `src/` and `shaders/` folders is my own work.

Third-party libraries (ImGui, GLFW, GLAD, GLM) are used under their respective licenses and credited to their original authors.
