#pragma once
#include <glm/glm.hpp>

struct NoiseSettings {
    float strength = 0.5f;
    float roughness = 2.1f;
    float baseRoughness = 1.0f;
    int octaves = 5;
    float persistence = 0.6f;
    float minValue = 1.1f;
    glm::vec3 center = glm::vec3(0.0f);
    bool enabled = true;

    NoiseSettings() = default;

    NoiseSettings(float s, float r, float baseR, int o, float p, float min, glm::vec3 c, bool e)
        : strength(s), roughness(r), baseRoughness(baseR), octaves(o), persistence(p), minValue(min), center(c), enabled(e) {}
};

float EvaluateNoise(const glm::vec3& point, const NoiseSettings& settings);
