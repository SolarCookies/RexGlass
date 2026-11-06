// What RexGlass keeps between its sources.

#pragma once

#include <unordered_set>

#include <imgui.h>

#include <rexglass/rexglass.h>

struct ImGuiContext;

namespace rexglass {

struct State {
  bool installed = false;
  Config config;
  // Windows a module asked to be glass this frame (BeginWindow, MarkWindow), by id.
  std::unordered_set<ImGuiID> marked;
  // The frame the backdrop was last asked for, and what it gave.
  int backdrop_frame = -1;
  ImTextureID backdrop = ImTextureID_Invalid;
  ImVec4 backdrop_uv{0.0f, 0.0f, 1.0f, 1.0f};
};

State& GetState();

// Puts a glass panel behind each glass window in this frame's draw data. Runs as ImGui::Render finishes, before the
// renderer draws the frame.
void DecorateWindows(ImGuiContext& context);

}  // namespace rexglass
