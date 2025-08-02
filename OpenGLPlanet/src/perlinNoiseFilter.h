
#pragma once
#include "noiseFilter.h"
#include "noiseLayer.h"

class PerlinNoiseFilter : public NoiseFilter {
public:
    PerlinNoiseFilter(const NoiseLayer& settings);
    virtual float Evaluate(const glm::vec3& point) const override;

private:
    NoiseLayer settings;
};
