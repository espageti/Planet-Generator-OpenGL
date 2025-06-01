#include "mesh.h"
#include "perlinNoiseFilter.h"
#include <glad/glad.h>
#include <array>


#include "globals.h"
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>


std::vector<glm::vec3> spherePositions;
std::vector<glm::vec3> sphereNormals;

std::vector<unsigned int> indices;


struct QuantizedVec3 {
    int x, y, z;
    bool operator==(const QuantizedVec3& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

namespace std {
    template <>
    struct hash<QuantizedVec3> {
        size_t operator()(const QuantizedVec3& v) const {
            return ((v.x * 73856093) ^ (v.y * 19349663) ^ (v.z * 83492791));
        }
    };
}


std::unordered_map<QuantizedVec3, int> vertexMap;

QuantizedVec3 Quantize(glm::vec3 v, int resolution) {
    float scale = resolution * 4.0f;  
    return {
        static_cast<int>(round(v.x * scale)),
        static_cast<int>(round(v.y * scale)),
        static_cast<int>(round(v.z * scale))
    };
}

int AddVertex(glm::vec3 pos, int resolution) {
    auto key = Quantize(pos, resolution);
    auto it = vertexMap.find(key);
    if (it != vertexMap.end()) return it->second;

    int index = spherePositions.size();
    vertexMap[key] = index;
    spherePositions.push_back(pos);
    sphereNormals.push_back(glm::vec3(0.0f));
    return index;
}


void GenerateSphere(std::vector<float>& verticesOut, std::vector<unsigned int>& indicesOut) {
    verticesOut.clear();
    indicesOut.clear();
    spherePositions.clear();
    sphereNormals.clear();
    vertexMap.clear();
    indices.clear();

    const int resolution = shape->resolution;
    const std::array<glm::vec3, 6> directions = {
        glm::vec3(0, 1, 0), glm::vec3(0, -1, 0),
        glm::vec3(-1, 0, 0), glm::vec3(1, 0, 0),
        glm::vec3(0, 0, 1), glm::vec3(0, 0, -1)
    };

    for (glm::vec3 localUp : directions) {
        glm::vec3 axisA = glm::vec3(localUp.y, localUp.z, localUp.x);
        glm::vec3 axisB = glm::cross(localUp, axisA);

        for (int y = 0; y < resolution; ++y) {
            for (int x = 0; x < resolution; ++x) {


                auto cubePoint = [resolution, localUp, axisA, axisB](int x, int y)
                {
                    float percentX = x / float(resolution - 1);
                    float percentY = y / float(resolution - 1);
                    return localUp +
                        (percentX - 0.5f) * 2.0f * axisA +
                        (percentY - 0.5f) * 2.0f * axisB;
                };
                glm::vec3 unitSpherePoint = glm::normalize(cubePoint(x, y));
                glm::vec3 finalPos = CalculatePointOnPlanet(unitSpherePoint);
                //glm::vec3 finalPos = unitSpherePoint * shape->radius;
                int idx = AddVertex(finalPos, shape->resolution);

                if (x < resolution - 1 && y < resolution - 1) {
                    int i0 = idx;
                    int i1 = AddVertex(CalculatePointOnPlanet(glm::normalize(
                        cubePoint(x + 1, y)
                    )), shape->resolution);
                    int i2 = AddVertex(CalculatePointOnPlanet(glm::normalize(
                        cubePoint(x, y + 1)
                    )), shape->resolution);
                    int i3 = AddVertex(CalculatePointOnPlanet(glm::normalize(
                        cubePoint(x + 1, y + 1)
                    )), shape->resolution);

                    indices.push_back(i0);
                    indices.push_back(i3);
                    indices.push_back(i2);

                    indices.push_back(i0);
                    indices.push_back(i1);
                    indices.push_back(i3);

                    //auto accumulate = [&](int a, int b, int c) {
                    //    glm::vec3 n = glm::normalize(glm::cross(
                    //        spherePositions[b] - spherePositions[a],
                    //        spherePositions[c] - spherePositions[a]));
                    //    sphereNormals[a] += n;
                    //    sphereNormals[b] += n;
                    //    sphereNormals[c] += n;
                    //};

                    //accumulate(i0, i3, i2); 
                    //accumulate(i0, i1, i3);
                }
            }
        }
    }

    // Output final interleaved vertex data
    for (int i = 0; i < spherePositions.size(); ++i) {
        glm::vec3 p = spherePositions[i];
        //glm::vec3 n = glm::normalize(sphereNormals[i]);

        verticesOut.push_back(p.x);
        verticesOut.push_back(p.y);
        verticesOut.push_back(p.z);
        //verticesOut.push_back(n.x);
        //verticesOut.push_back(n.y);
        //verticesOut.push_back(n.z);
    }

    indicesOut = indices;
}

glm::vec3 CalculatePointOnPlanet(glm::vec3 pointOnUnitSphere)
{
    return pointOnUnitSphere;
    //we have now moved this stuff to the GPU
    float elevation = 0;
    for (const auto& settings: shape->noiseLayers)
    {
        if (!settings->enabled)
        {
            continue;
        }
        PerlinNoiseFilter* filter = new PerlinNoiseFilter(*settings);
        elevation += filter->Evaluate(pointOnUnitSphere);

        delete filter;
    }
    return pointOnUnitSphere * shape->radius * (1 + elevation);
}

void UploadMesh(unsigned int& VAO, unsigned int& VBO, unsigned int& EBO,
    const std::vector<float>& vertices, const std::vector<unsigned int>& indices) {
    // Delete old buffers if they exist
    if (VAO != 0) glDeleteVertexArrays(1, &VAO);
    if (VBO != 0) glDeleteBuffers(1, &VBO);
    if (EBO != 0) glDeleteBuffers(1, &EBO);

    // Generate new buffers
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal attribute
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    //glEnableVertexAttribArray(1);
    //glBindVertexArray(0);
}
