#pragma once
#include <filesystem>
#include "shapeSettings.h"
#include <glm/glm.hpp>

extern ShapeSettings* shape;

extern float rotationSpeed;
extern float densityFalloff;
extern float gMie; // The Mie phase asymmetry factor ( > 0 means forward scattering, < 0 means backward scattering)
extern bool atmosphereEnabled;
extern bool firstPersonMode;

extern glm::vec3 lightColor;