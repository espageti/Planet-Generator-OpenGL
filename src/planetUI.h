#pragma once
#include "engine.h" 
#include <imgui.h>
#include <functional>

namespace PlanetUI {
    bool DrawNoiseLayerControls(ShapeSettings* shape);
    void DrawMainControls(ShapeSettings* shape, std::function<void()> onRegenerate);
    void DrawSaveLoadControls(ShapeSettings* shape);
}