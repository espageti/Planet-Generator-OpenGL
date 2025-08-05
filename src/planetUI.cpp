#include "planetUI.h"
#include "globals.h"
#include <fstream>
#include <filesystem>
#include <algorithm>

namespace PlanetUI {

    bool DrawNoiseLayerControls(ShapeSettings* shape) {
        bool changed = false;
        int i = 0;
        for (auto it = shape->noiseLayers.begin(); it != shape->noiseLayers.end(); ) {
            NoiseLayer* layer = *it;
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
            changed |= ImGui::Checkbox("Enabled", &layer->enabled);

            if (layer->enabled) {
                changed |= ImGui::SliderFloat("Strength", &layer->strength, 0.0f, 2.0f);
                changed |= ImGui::SliderFloat("Roughness", &layer->roughness, 0.0f, 5.0f);
                changed |= ImGui::SliderFloat("Base Roughness", &layer->baseRoughness, 0.0f, 5.0f);
                changed |= ImGui::SliderInt("Octaves", &layer->octaves, 1, 10);
                changed |= ImGui::SliderFloat("Persistence", &layer->persistence, 0.0f, 1.0f);
                changed |= ImGui::SliderFloat("Min Value", &layer->minValue, 0.0f, 2.0f);
            }

            ImGui::SameLine();
            if (ImGui::Button("Delete")) {
                delete layer;
                it = shape->noiseLayers.erase(it);
                ImGui::PopID();
                changed = true;
                continue;
            }
            else {
                it++;
            }

            ImGui::PopID();
            i++;
        }

        
        if (ImGui::Button("Add New Layer")) {
            shape->noiseLayers.push_back(new NoiseLayer());
            changed = true;
        }
        return true;
    }

    void DrawSaveLoadControls(ShapeSettings* shape) {
        static char foldername[256] = "planets";
        static char filename[256] = "planet_config.txt";

        ImGui::InputText("Folder", foldername, sizeof(foldername));
        ImGui::InputText("Filename", filename, sizeof(filename));

        if (ImGui::Button("Save Config")) {
            namespace fs = std::filesystem;
            fs::path dir = foldername;
            fs::path file_path = filename; 
            fs::path fullpath = dir / file_path;

            // Create directory if it doesn't exist
            if (!fs::exists(dir)) {
                if (!fs::create_directories(dir)) {
                    ImGui::TextColored(ImVec4(1, 0, 0, 1), "Failed to create directory!");
                    return;
                }
            }

            std::ofstream out_file(fullpath);  
            if (out_file) { 
                out_file << shape->Serialize();
                ImGui::TextColored(ImVec4(0, 1, 0, 1), "Config saved successfully!");
            }
            else {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "Failed to open file for writing!");
            }
        }

        if (ImGui::Button("Load Config")) {
            namespace fs = std::filesystem;
            fs::path dir = foldername;
            fs::path file_path = filename; 
            fs::path fullpath = dir / file_path;

            std::ifstream in_file(fullpath);
            if (in_file) { 
                std::string content((std::istreambuf_iterator<char>(in_file)),
                    std::istreambuf_iterator<char>());
                shape->Deserialize(content);
                ImGui::TextColored(ImVec4(0, 1, 0, 1), "Config loaded successfully!");
            }
            else {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "Failed to open file for reading!");
            }
        }
    }

    bool autoRegen = true;
    void DrawMainControls(ShapeSettings* shape, std::function<void()> onRegenerate) {
        ImGui::Begin("Planet Editor");
        //ImGui::SliderFloat("Planet Radius", &shape->radius, 0.0f, 10.0f);
        ImGui::SliderFloat("Rotation Speed", &rotationSpeed, 0.0, 3.0f);
        ImGui::SliderFloat("Density Falloff", &densityFalloff, 0.0, 30.0f);
        
        bool seedChanged = ImGui::SliderFloat("Terrain Seed", &shape->seed, 0.0f, 100.0f);
        if (seedChanged && autoRegen) {
            onRegenerate();
        }
        
        ImGui::Checkbox("Atmosphere", &atmosphereEnabled);
        ImGui::Checkbox("First Person", &firstPersonMode);
        ImGui::SliderFloat("G Mie", &gMie, -1.0f, 1.0f);
		ImGui::ColorEdit3("Light Color", (float*) &lightColor);
        bool update = DrawNoiseLayerControls(shape);
        ImGui::Checkbox("Auto Regenerate", &autoRegen);

        
        if ((autoRegen && update) || ImGui::Button("Regenerate")) {
            onRegenerate();
        }

        DrawSaveLoadControls(shape);

        ImGui::End();
    }

} 