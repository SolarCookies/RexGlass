#include <rexglass/rexglass.h>

#include <algorithm>

#include <imgui_internal.h>

#include <rex/ui/imgui_drawer.h>

#include "state.h"

namespace rexglass {

State& GetState() {
  static State state;
  return state;
}

void Install(rex::ui::ImGuiDrawer* drawer, Config config) {
  State& state = GetState();
  if (state.installed || !drawer) return;
  state.config = std::move(config);
  state.installed = true;

  // GetIO makes the drawer's context current, which the hook goes on.
  drawer->GetIO();
  ImGuiContextHook hook;
  hook.Type = ImGuiContextHookType_RenderPost;
  hook.Owner = 0;
  hook.Callback = [](ImGuiContext* context, ImGuiContextHook*) { DecorateWindows(*context); };
  ImGui::AddContextHook(ImGui::GetCurrentContext(), &hook);

  if (state.config.theme) {
    ApplyTheme(state.config.scale, state.config.accent);
  } else if (state.config.all_windows) {
    // The game's own look otherwise, but window backgrounds have to be see-through for the glass to show.
    ImVec4* colors = ImGui::GetStyle().Colors;
    for (ImGuiCol color : {ImGuiCol_WindowBg, ImGuiCol_PopupBg, ImGuiCol_TitleBg, ImGuiCol_TitleBgCollapsed, ImGuiCol_MenuBarBg}) colors[color].w = 0.0f;
    colors[ImGuiCol_TitleBgActive].w = std::min(colors[ImGuiCol_TitleBgActive].w, 0.1f);
  }
}

bool Installed() { return GetState().installed; }

const Config& GetConfig() { return GetState().config; }

void SetStyle(const WindowStyle& style) { GetState().config.style = style; }

ImTextureID Backdrop(ImVec4* uv) {
  State& state = GetState();
  if (!state.installed || !state.config.backdrop) return ImTextureID_Invalid;
  // Asked once a frame: the game's renderer may blur on demand, and the image lasts the frame.
  const int frame = ImGui::GetFrameCount();
  if (state.backdrop_frame != frame) {
    state.backdrop_frame = frame;
    float corners[4] = {0.0f, 0.0f, 1.0f, 1.0f};
    rex::ui::ImmediateTexture* image = state.config.backdrop(state.config.blur * state.config.scale, corners);
    state.backdrop = image ? reinterpret_cast<ImTextureID>(image) : ImTextureID_Invalid;
    state.backdrop_uv = ImVec4(corners[0], corners[1], corners[2], corners[3]);
  }
  if (uv) *uv = state.backdrop_uv;
  return state.backdrop;
}

bool BeginWindow(const char* name, bool* open, ImGuiWindowFlags flags) {
  // See-through, so the glass behind shows; the glass draws the edge and the title band.
  const ImVec4 clear(0.0f, 0.0f, 0.0f, 0.0f);
  ImGui::PushStyleColor(ImGuiCol_WindowBg, clear);
  ImGui::PushStyleColor(ImGuiCol_TitleBg, clear);
  ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(1.0f, 1.0f, 1.0f, 0.04f));
  ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, clear);
  ImGui::PushStyleColor(ImGuiCol_MenuBarBg, clear);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  const bool shown = ImGui::Begin(name, open, flags);
  ImGui::PopStyleVar();
  ImGui::PopStyleColor(5);
  MarkWindow();
  return shown;
}

void MarkWindow() {
  if (const ImGuiWindow* window = ImGui::GetCurrentWindowRead()) GetState().marked.insert(window->ID);
}

}  // namespace rexglass
