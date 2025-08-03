#pragma once
#include <filesystem>
#include "shapeSettings.h"
#include <glm/glm.hpp>

extern ShapeSettings* shape;

extern float rotationSpeed;
extern float densityFalloff;
extern bool atmosphereEnabled;
extern bool firstPersonMode;

extern glm::vec3 lightColor;