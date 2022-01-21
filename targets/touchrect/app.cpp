#include "app.h"

#include "imgui.h"
#include "pointer_injection.h"

using namespace ImGui;

app::app(HWND window) : main_window(window)
{
}

void app::draw_gui()
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking
        | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoBringToFrontOnFocus
        | ImGuiWindowFlags_NoNavFocus;

    ImGuiViewport* viewport = GetMainViewport();
    SetNextWindowPos(viewport->WorkPos);
    SetNextWindowSize(viewport->WorkSize);
    SetNextWindowViewport(viewport->ID);
    PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    bool open = true;
    Begin("Main", &open, window_flags);

    PopStyleVar(3);
    
    if (show_imgui_metrics) { ShowMetricsWindow(&show_imgui_metrics); }
    if (show_imgui_about) { ShowAboutWindow(&show_imgui_about); }
    if (show_style_editor)
    {
        Begin("Dear ImGui Style Editor", &show_style_editor);
        ShowStyleEditor();
        End();
    }
    
    ImGuiID dockspace_id = GetID("MainDockSpace");
    DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    SetNextWindowDockID(dockspace_id, ImGuiCond_Once);

    // your window here instead of the Demo
    // ShowDemoWindow(&open);

    for (auto& pointer : present_pointers)
    {
        pointer.second.draw_gui();
    }

    pointer_injection::singleton.inject(present_pointers);

    previous_pointers = present_pointers;
    present_pointers.clear();

    if (BeginMainMenuBar())
    {
        if (BeginMenu("File"))
        {
            if (MenuItem("Exit"))
            {
                ::PostQuitMessage(0);
            }
            ImGui::EndMenu();
        }
        if (BeginMenu("Tools"))
        {
            if (BeginMenu("ImGui"))
            {
                MenuItem("Metrics/Debugger", NULL, &show_imgui_metrics);
                MenuItem("Style Editor", NULL, &show_style_editor);
                MenuItem("About Dear ImGui", NULL, &show_imgui_about);
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        EndMainMenuBar();
    }
    End();
}

std::optional<LRESULT> app::wndproc(std::optional<LRESULT> previous, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    std::optional<LRESULT> result;
    if (hWnd == main_window)
    {
        switch (msg)
        {
        case WM_POINTERENTER:
        case WM_POINTERLEAVE:
        case WM_POINTERDOWN:
        case WM_POINTERUPDATE:
        case WM_POINTERUP:
        case WM_POINTERWHEEL:
        case WM_POINTERHWHEEL:
        {
            auto pid = GET_POINTERID_WPARAM(wParam);

            auto pointer = previous_pointers.contains(pid)
                ? user_pointer({ hWnd, msg, wParam, lParam }, previous_pointers[pid])
                : user_pointer({ hWnd, msg, wParam, lParam });

            present_pointers.insert_or_assign(pid, pointer);

            if (pointer.type == PT_MOUSE) {
                result = DefWindowProc(hWnd, msg, wParam, lParam);
            }
            result = 0;
        }
        }
    }
    return result;
}
