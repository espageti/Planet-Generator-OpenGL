#pragma once
#include <glm/glm.hpp>
#include "noiseSettings.h"

class NoiseFilter {
public:
    virtual float Evaluate(const glm::vec3& point) const = 0;
    virtual ~NoiseFilter() = default;
};
