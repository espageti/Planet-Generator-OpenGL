#pragma once
#include <vector>
#include <sstream>
#include "noiseSettings.h"
struct ShapeSettings {
    float radius = 1.0f;
    int resolution = 40;
    std::vector<NoiseSettings*> noiseLayers;

    ShapeSettings() = default;

    ShapeSettings(float r, int res)
        : radius(r), resolution(res) {}


    std::string Serialize() {
        std::ostringstream ss;
        ss << radius << " " << noiseLayers.size() << "\n";
        for (auto layer : noiseLayers) {
            ss << layer->Serialize() << "\n";
        }
        return ss.str();
    }

    void Deserialize(const std::string& data) {
        std::istringstream ss(data);
        size_t layerCount;
        ss >> radius >> layerCount;

        // Clear existing layers
        for (auto layer : noiseLayers) delete layer;
        noiseLayers.clear();

        // Read layers
        for (size_t i = 0; i < layerCount; i++) {
            NoiseSettings* layer = new NoiseSettings();
            std::string layerData;
            std::getline(ss >> std::ws, layerData); // Read whole line
            layer->Deserialize(layerData);
            noiseLayers.push_back(layer);
        }
    }
    
    void AddNoiseLayer(NoiseSettings* layer);
};
