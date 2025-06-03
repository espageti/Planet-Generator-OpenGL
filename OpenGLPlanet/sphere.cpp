// sphere.cpp
#include "sphere.h"
#include <glad/glad.h>
#include "globals.h"

const std::array<glm::vec3, 6> Sphere::m_Directions = {
    glm::vec3(0, 1, 0), glm::vec3(0, -1, 0),
    glm::vec3(-1, 0, 0), glm::vec3(1, 0, 0),
    glm::vec3(0, 0, 1), glm::vec3(0, 0, -1)
};

Sphere::Sphere() = default;

Sphere::~Sphere() {
    Destroy();
}

bool Sphere::Create(float radius, int resolution) {
    Destroy();

    m_fRadius = radius;
    m_nResolution = resolution;

    GenerateGeometry();

    // Generate and bind VAO/VBO/EBO
    glGenVertexArrays(1, &m_nVAO);
    glGenBuffers(1, &m_nVBO);
    glGenBuffers(1, &m_nEBO);

    glBindVertexArray(m_nVAO);

    // Position data
    glBindBuffer(GL_ARRAY_BUFFER, m_nVBO);
    glBufferData(GL_ARRAY_BUFFER, m_vPositions.size() * sizeof(glm::vec3), m_vPositions.data(), GL_STATIC_DRAW);

    // Index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_nEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_vIndices.size() * sizeof(unsigned int), m_vIndices.data(), GL_STATIC_DRAW);

    // Vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    m_nIndexCount = m_vIndices.size();
    return true;
}

void Sphere::Destroy() {
    if (m_nVAO) {
        glDeleteVertexArrays(1, &m_nVAO);
        glDeleteBuffers(1, &m_nVBO);
        glDeleteBuffers(1, &m_nEBO);
        m_nVAO = m_nVBO = m_nEBO = 0;
    }
}

void Sphere::Draw() const {
    if (m_nVAO) {
        glBindVertexArray(m_nVAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)m_nIndexCount, GL_UNSIGNED_INT, 0);
    }
}

void Sphere::GenerateGeometry() {
    m_vPositions.clear();
    m_vNormals.clear();
    m_vIndices.clear();
    m_VertexMap.clear();

    for (const glm::vec3& localUp : m_Directions) {
        glm::vec3 axisA(localUp.y, localUp.z, localUp.x);
        glm::vec3 axisB = glm::cross(localUp, axisA);

        for (int y = 0; y < m_nResolution; ++y) {
            for (int x = 0; x < m_nResolution; ++x) {
                auto cubePoint = [this, localUp, axisA, axisB](int x, int y) {
                    float percentX = x / float(m_nResolution - 1);
                    float percentY = y / float(m_nResolution - 1);
                    return localUp +
                        (percentX - 0.5f) * 2.0f * axisA +
                        (percentY - 0.5f) * 2.0f * axisB;
                };

                glm::vec3 unitSpherePoint = glm::normalize(cubePoint(x, y));
                glm::vec3 finalPos = CalculatePointOnPlanet(unitSpherePoint);
                unsigned int idx = AddVertex(finalPos);

                if (x < m_nResolution - 1 && y < m_nResolution - 1) {
                    unsigned int i0 = idx;
                    unsigned int i1 = AddVertex(CalculatePointOnPlanet(glm::normalize(cubePoint(x + 1, y))));
                    unsigned int i2 = AddVertex(CalculatePointOnPlanet(glm::normalize(cubePoint(x, y + 1))));
                    unsigned int i3 = AddVertex(CalculatePointOnPlanet(glm::normalize(cubePoint(x + 1, y + 1))));

                    m_vIndices.insert(m_vIndices.end(), { i0, i3, i2 });
                    m_vIndices.insert(m_vIndices.end(), { i0, i1, i3 });
                }
            }
        }
    }
}

Sphere::QuantizedVec3 Sphere::Quantize(glm::vec3 v) const {
    float scale = m_nResolution * 4.0f;
    return {
        static_cast<int>(round(v.x * scale)),
        static_cast<int>(round(v.y * scale)),
        static_cast<int>(round(v.z * scale))
    };
}

unsigned int Sphere::AddVertex(glm::vec3 pos) {
    auto key = Quantize(pos);
    auto it = m_VertexMap.find(key);
    if (it != m_VertexMap.end()) return it->second;

    int index = static_cast<int>(m_vPositions.size());
    m_VertexMap[key] = index;
    m_vPositions.push_back(pos);
    m_vNormals.push_back(glm::vec3(0.0f));
    return index;
}

glm::vec3 Sphere::CalculatePointOnPlanet(glm::vec3 pointOnUnitSphere) const {
    // Your existing terrain generation logic
    return pointOnUnitSphere * m_fRadius;
}