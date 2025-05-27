#pragma once
#include <vector>
#include "noiseSettings.h"
struct ShapeSettings {
    float radius = 1.0f;
    int resolution = 40;
    std::vector<NoiseSettings*> noiseLayers;

    ShapeSettings() = default;

    ShapeSettings(float r, int res)
        : radius(r), resolution(res) {}
    
    void AddNoiseLayer(NoiseSettings* layer);
};
