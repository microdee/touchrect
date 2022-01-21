
#pragma once

#include <Windows.h>
#include <optional>
#include <unordered_map>

#include "user_pointer.h"

class app
{
public:
    app(HWND window);
    void draw_gui();
    std::optional<LRESULT> wndproc(std::optional<LRESULT> previous, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    bool show_imgui_metrics = false;
    bool show_style_editor = false;
    bool show_imgui_about = false;
    HWND main_window = NULL;

    std::unordered_map<WORD, user_pointer> present_pointers;
    std::unordered_map<WORD, user_pointer> previous_pointers;
};