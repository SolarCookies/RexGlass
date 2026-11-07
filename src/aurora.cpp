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

// Where a light is at `seconds`, drifting around the area.
ImVec2 LightCenter(const Light& light, const ImVec2& min, const ImVec2& size, float seconds) { return ImVec2(min.x + size.x * (0.5f + 0.42f * std::sin(seconds * light.speed_x + light.phase)), min.y + size.y * (0.5f + 0.42f * std::cos(seconds * light.speed_y + light.phase * 1.3f))); }

// The deep blue the lights drift over, from the top left corner clockwise.
constexpr ImU32 kCorners[4] = {IM_COL32(16, 18, 34, 255), IM_COL32(20, 16, 36, 255), IM_COL32(8, 10, 20, 255), IM_COL32(10, 9, 22, 255)};

}  // namespace

void DrawAurora(ImDrawList* draw, const ImVec2& min, const ImVec2& max, float seconds, float opacity) {
  const ImVec2 size(max.x - min.x, max.y - min.y);
  if (!draw || size.x <= 0.0f || size.y <= 0.0f || opacity <= 0.0f) return;
  draw->PushClipRect(min, max, true);
  draw->AddRectFilledMultiColor(min, max, WithOpacity(kCorners[0], opacity), WithOpacity(kCorners[1], opacity), WithOpacity(kCorners[2], opacity), WithOpacity(kCorners[3], opacity));
  const float larger = std::max(size.x, size.y);
  for (const Light& light : kLights) AddRadialGradient(draw, LightCenter(light, min, size, seconds), larger * light.size, WithOpacity(ImGui::ColorConvertFloat4ToU32(light.color), opacity));
  draw->PopClipRect();
}

ImU32 AuroraColor(const ImVec2& min, const ImVec2& max, float seconds, const ImVec2& point) {
  const ImVec2 size(std::max(max.x - min.x, 1.0f), std::max(max.y - min.y, 1.0f));
  const float u = std::clamp((point.x - min.x) / size.x, 0.0f, 1.0f);
  const float v = std::clamp((point.y - min.y) / size.y, 0.0f, 1.0f);
  ImVec4 corner[4];
  for (int i = 0; i < 4; ++i) corner[i] = ImGui::ColorConvertU32ToFloat4(kCorners[i]);
  const auto mix = [](const ImVec4& a, const ImVec4& b, float t) { return ImVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, 1.0f); };
  ImVec4 color = mix(mix(corner[0], corner[1], u), mix(corner[3], corner[2], u), v);

  // Each light over it as DrawAurora blends it: its color fading linearly to nothing at its radius.
  const float larger = std::max(size.x, size.y);
  for (const Light& light : kLights) {
    const ImVec2 center = LightCenter(light, min, size, seconds);
    const float distance = std::sqrt((point.x - center.x) * (point.x - center.x) + (point.y - center.y) * (point.y - center.y));
    const float alpha = light.color.w * std::max(0.0f, 1.0f - distance / (larger * light.size));
    color = mix(color, light.color, alpha);
  }
  return ImGui::ColorConvertFloat4ToU32(color);
}

}  // namespace rexglass
