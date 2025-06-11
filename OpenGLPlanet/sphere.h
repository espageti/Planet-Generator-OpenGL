// sphere.h
#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include <array>

class Sphere {
public:
    Sphere();
    ~Sphere();

    bool Create(float radius, int resolution);
    void Destroy();
    void Draw() const;

    float GetRadius() const { return m_fRadius; }
    int GetResolution() const { return m_nResolution; }

private:
    struct QuantizedVec3 {
        int x, y, z;
        bool operator==(const QuantizedVec3& other) const {
            return x == other.x && y == other.y && z == other.z;
        }
    };

    struct QuantizedVec3Hash {
        size_t operator()(const QuantizedVec3& v) const {
            return ((v.x * 73856093) ^ (v.y * 19349663) ^ (v.z * 83492791));
        }
    };

    void GenerateGeometry();
    QuantizedVec3 Quantize(glm::vec3 v) const;
    unsigned int AddVertex(glm::vec3 pos);
    glm::vec3 CalculatePointOnPlanet(glm::vec3 pointOnUnitSphere) const;

    unsigned int m_nVAO = 0;
    unsigned int m_nVBO = 0;
    unsigned int m_nEBO = 0;
    size_t m_nIndexCount = 0;
    float m_fRadius = 1.0f;
    int m_nResolution = 16;

    // Temporary generation state
    std::vector<glm::vec3> m_vPositions;
    std::vector<glm::vec3> m_vNormals;
    std::vector<unsigned int> m_vIndices;
    std::unordered_map<QuantizedVec3, int, QuantizedVec3Hash> m_VertexMap;

    static const std::array<glm::vec3, 6> m_Directions;
};