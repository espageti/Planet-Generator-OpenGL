#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "shapeSettings.h"


void UploadMesh(unsigned int& VAO, unsigned int& VBO, unsigned int& EBO,
    const std::vector<float>& vertices, const std::vector<unsigned int>& indices);

glm::vec3 CalculatePointOnPlanet(glm::vec3 pointOnUnitSphere);
int AddVertex(glm::vec3 pos, int resolution);
void GenerateSphere(std::vector<float>& verticesOut, std::vector<unsigned int>& indicesOut);