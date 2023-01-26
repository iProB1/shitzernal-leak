#include "imwrap.h"




bool imwrap::ToggleButton(const char* label, bool* v, const ImVec2& size_arg, int style)
{
    return false;
}

void imwrap::tooltip(const char* tip) {
    if (tip) {
        if (ImGui::IsItemHovered() && GImGui->HoveredIdTimer > 0.2f) //we do not want to hover when mouse just passes by the item therefore delay
            ImGui::SetTooltip(tip);
    }
}

void imwrap::prep_columns(int count) {
    auto& style = ImGui::GetStyle();
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ style.WindowPadding.x, style.ItemSpacing.y });
    ImGui::Columns(count, nullptr, false);
    float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;
    float col_w = group_w / (float)count;

    for (int i = 1; i <= count; i++)
        ImGui::SetColumnOffset(i, i * col_w);
}
