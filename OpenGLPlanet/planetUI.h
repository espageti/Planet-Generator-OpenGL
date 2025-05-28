#pragma once
#include "engine.h" 
#include <imgui/imgui.h>

namespace PlanetUI {
    void DrawNoiseLayerControls(ShapeSettings* shape);
    void DrawMainControls(ShapeSettings* shape, std::function<void()> onRegenerate);
}