
#pragma once
#include "noiseFilter.h"
#include "noiseSettings.h"

class PerlinNoiseFilter : public NoiseFilter {
public:
    PerlinNoiseFilter(const NoiseSettings& settings);
    virtual float Evaluate(const glm::vec3& point) const override;

private:
    NoiseSettings settings;
};
