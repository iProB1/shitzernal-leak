#pragma once
#include <menu/menu.h>
#include <string>
#include <menu\imgui\imgui_internal.h>



struct InputTextCallback_UserData {
    std::string* Str;
    ImGuiInputTextCallback ChainCallback;
    void* ChainCallbackUserData;
};

#define BLOCK_SIZE 30.0f
#define AUTOSIZE(x) ImVec2(ImGui::GetWindowWidth() * 0.93f, BLOCK_SIZE * x)
#define AUTOSIZEC(x) ImVec2(ImGui::GetColumnWidth() - 10.f, BLOCK_SIZE * x)
#define RGBA(r, g, b, a) (((unsigned char)(a)) + ((unsigned char)(r) << 8) + ((unsigned char)(g) << 16) + ((unsigned char)(b) << 24))

namespace imwrap {

    //from private internal, might be used later on, so bringing it for now
    namespace col {
        struct im {
            float red;
            float green;
            float blue;
            float alpha;
        };

        //GT argb
        int im_to_gt(im& c);
        im gt_to_im(int gt);
    }


    static auto vector_getter = [](void* vec, int idx, const char** out_text) {
        auto& vector = *static_cast<std::vector<std::string>*>(vec);
        if (idx < 0 || idx >= static_cast<int>(vector.size())) {
            return false;
        }
        *out_text = vector.at(idx).c_str();
        return true;
    };

    IMGUI_API bool ToggleButton(const char* label, bool* v, const ImVec2& size_arg = ImVec2(0, 0), int style = 0);

    void tooltip(const char* tip = nullptr);

    void prep_columns(int count);


    template <size_t N>
    void horizontal_tabs(char* (&names)[N], int& activetab, float w, float h) {
        bool values[N] = { false };
        values[activetab] = true;
        for (int i = 0; i < N; ++i) {
            if (ToggleButton(names[i], &values[i], ImVec2{ w, h }, 1))
                activetab = i;

            if (i < N - 1)
                ImGui::SameLine();
        }
    }

} // namespace imwrap