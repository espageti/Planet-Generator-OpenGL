
#include "perlinNoiseFilter.h"
#include <glm/gtc/noise.hpp>
#include <algorithm>

PerlinNoiseFilter::PerlinNoiseFilter(const NoiseSettings& settings) : settings(settings) {}

float PerlinNoiseFilter::Evaluate(const glm::vec3& point) const {
    float noiseValue = 0.0f;
    float frequency = settings.baseRoughness;
    float amplitude = 1.0f;
    glm::vec3 p = point + settings.center;

    for (int i = 0; i < settings.octaves; i++) {
        float v = glm::perlin(p * frequency);
        noiseValue += (v + 1.0f) * 0.5f * amplitude;

        frequency *= settings.roughness;
        amplitude *= settings.persistence;
    }

    noiseValue = std::max(0.0f, noiseValue - settings.minValue);
    return noiseValue * settings.strength;
}
