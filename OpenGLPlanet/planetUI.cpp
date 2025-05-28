#include "planetUI.h"
#include <algorithm> // for std::iter_swap

namespace PlanetUI {

    void DrawNoiseLayerControls(ShapeSettings* shape) {
        int i = 0;
        for (auto it = shape->noiseLayers.begin(); it != shape->noiseLayers.end(); ) {
            NoiseSettings* layer = *it;
            ImGui::PushID(i);

            ImGui::Separator();

            // Drag and drop for reordering
            ImGui::Text("Layer %d", i);
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                ImGui::SetDragDropPayload("NOISE_LAYER", &i, sizeof(int));
                ImGui::Text("Move layer %d", i);
                ImGui::EndDragDropSource();
            }

            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("NOISE_LAYER")) {
                    IM_ASSERT(payload->DataSize == sizeof(int));
                    int payload_i = *(const int*)payload->Data;
                    if (payload_i != i) {
                        std::iter_swap(shape->noiseLayers.begin() + payload_i,
                            shape->noiseLayers.begin() + i);
                    }
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::SameLine();
            ImGui::Checkbox("Enabled", &layer->enabled);

            if (layer->enabled) {
                ImGui::SliderFloat("Strength", &layer->strength, 0.0f, 10.0f);
                ImGui::SliderFloat("Roughness", &layer->roughness, 0.0f, 5.0f);
                ImGui::SliderFloat("Base Roughness", &layer->baseRoughness, 0.0f, 5.0f);
                ImGui::SliderInt("Octaves", &layer->octaves, 1, 10);
                ImGui::SliderFloat("Persistence", &layer->persistence, 0.0f, 1.0f);
                ImGui::SliderFloat("Min Value", &layer->minValue, 0.0f, 2.0f);
            }

            ImGui::SameLine();
            if (ImGui::Button("Delete")) {
                delete layer;
                it = shape->noiseLayers.erase(it);
                ImGui::PopID();
                continue;
            }
            else {
                it++;
            }

            ImGui::PopID();
            i++;
        }

        if (ImGui::Button("Add New Layer")) {
            shape->noiseLayers.push_back(new NoiseSettings());
        }
    }

    void DrawMainControls(ShapeSettings* shape, std::function<void()> onRegenerate) {
        ImGui::Begin("Planet Editor");
        ImGui::SliderFloat("Planet Radius", &shape->radius, 0.0f, 10.0f);

        DrawNoiseLayerControls(shape);

        if (ImGui::Button("Regenerate")) {
            onRegenerate();
        }

        ImGui::End();
    }

} // namespace PlanetUI