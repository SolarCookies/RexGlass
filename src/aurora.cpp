// A backdrop of colored light drifting slowly over a deep blue, for canvases under glass. From the Toon engine's
// node graph editors.

#include <algorithm>
#include <array>
#include <cmath>

#include <rexglass/rexglass.h>

namespace rexglass {
namespace {

struct Light {
  ImVec4 color;
  float speed_x;
  float speed_y;
  float phase;
  float size;  // a fraction of the larger side of the area
};

constexpr std::array<Light, 5> kLights = {{
    {{0.15f, 0.35f, 1.0f, 0.55f}, 0.11f, 0.07f, 0.0f, 0.55f},
    {{0.75f, 0.25f, 0.95f, 0.42f}, 0.07f, 0.13f, 1.7f, 0.5f},
    {{0.05f, 0.75f, 0.8f, 0.38f}, 0.09f, 0.05f, 3.1f, 0.45f},
    {{0.45f, 0.3f, 1.0f, 0.4f}, 0.05f, 0.1f, 4.4f, 0.6f},
    {{1.0f, 0.45f, 0.35f, 0.25f}, 0.13f, 0.08f, 5.6f, 0.35f},
}};

ImU32 WithOpacity(ImU32 color, float opacity) {
  const float alpha = static_cast<float>((color >> IM_COL32_A_SHIFT) & 0xFF) * opacity;
  return (color & ~IM_COL32_A_MASK) | (static_cast<ImU32>(alpha) << IM_COL32_A_SHIFT);
}

// A disc fading from `color` at the center to nothing at the edge.
void AddRadialGradient(ImDrawList* draw, const ImVec2& center, float radius, ImU32 color) {
  constexpr int kSegments = 48;
  const ImU32 clear = color & ~IM_COL32_A_MASK;
  const ImVec2 uv = ImGui::GetFontTexUvWhitePixel();
  draw->PrimReserve(kSegments * 3, kSegments + 1);
  const unsigned int base = draw->_VtxCurrentIdx;
  draw->PrimWriteVtx(center, uv, color);
  for (int segment = 0; segment < kSegments; ++segment) {
    const float angle = 6.2831853f * static_cast<float>(segment) / static_cast<float>(kSegments);
    draw->PrimWriteVtx(ImVec2(center.x + std::cos(angle) * radius, center.y + std::sin(angle) * radius), uv, clear);
  }
  for (int segment = 0; segment < kSegments; ++segment) {
    draw->PrimWriteIdx(static_cast<ImDrawIdx>(base));
    draw->PrimWriteIdx(static_cast<ImDrawIdx>(base + 1 + static_cast<unsigned int>(segment)));
    draw->PrimWriteIdx(static_cast<ImDrawIdx>(base + 1 + static_cast<unsigned int>((segment + 1) % kSegments)));
  }
}

}  // namespace

void DrawAurora(ImDrawList* draw, const ImVec2& min, const ImVec2& max, float seconds, float opacity) {
  const ImVec2 size(max.x - min.x, max.y - min.y);
  if (!draw || size.x <= 0.0f || size.y <= 0.0f || opacity <= 0.0f) return;
  draw->PushClipRect(min, max, true);
  draw->AddRectFilledMultiColor(min, max, WithOpacity(IM_COL32(16, 18, 34, 255), opacity), WithOpacity(IM_COL32(20, 16, 36, 255), opacity), WithOpacity(IM_COL32(8, 10, 20, 255), opacity), WithOpacity(IM_COL32(10, 9, 22, 255), opacity));
  const float larger = std::max(size.x, size.y);
  for (const Light& light : kLights) {
    const ImVec2 center(min.x + size.x * (0.5f + 0.42f * std::sin(seconds * light.speed_x + light.phase)), min.y + size.y * (0.5f + 0.42f * std::cos(seconds * light.speed_y + light.phase * 1.3f)));
    AddRadialGradient(draw, center, larger * light.size, WithOpacity(ImGui::ColorConvertFloat4ToU32(light.color), opacity));
  }
  draw->PopClipRect();
}

}  // namespace rexglass
