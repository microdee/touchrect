#include "app.h"

#include "imgui.h"
#include "simulated_output.h"

app::app(HWND window) : main_window(window)
{
}

void app::draw_gui()
{
    namespace ui = ImGui;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking
        | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoBringToFrontOnFocus
        | ImGuiWindowFlags_NoNavFocus;

    ImGuiViewport* viewport = ui::GetMainViewport();
    ui::SetNextWindowPos(viewport->WorkPos);
    ui::SetNextWindowSize(viewport->WorkSize);
    ui::SetNextWindowViewport(viewport->ID);
    ui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    bool open = true;
    ui::Begin("Main", &open, window_flags);

    ui::PopStyleVar(3);
    
    if (show_imgui_metrics) { ui::ShowMetricsWindow(&show_imgui_metrics); }
    if (show_imgui_about) { ui::ShowAboutWindow(&show_imgui_about); }
    if (show_imgui_demo) { ui::ShowDemoWindow(&show_imgui_demo); }
    if (show_style_editor)
    {
        if(ui::Begin("Dear ImGui Style Editor", &show_style_editor))
        {
            ui::ShowStyleEditor();
        }
        ui::End();
    }
    if(show_about)
    {
        if(ui::Begin("About touchrect", &show_about, ImGuiWindowFlags_Modal))
        {
            ui::Text("touchrect v0.0.1");
            ui::Text("Made by David Mórász");
            ui::Text("MIT license");
        }
        ui::End();
    }
    
    ImGuiID dockspace_id = ui::GetID("MainDockSpace");
    ui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    ui::SetNextWindowDockID(dockspace_id, ImGuiCond_Once);

    for (auto& pointer : present_pointers)
    {
        pointer.second.draw_gui();
    }

    simulated_output::singleton.simulate(present_pointers);

    if(show_settings)
    {
        if(ui::Begin("Settings", &show_settings))
        {
            static std::vector<std::string> settings
            {
                "Output",
                "Appearance"
            };

            ui::BeginChild("Categories", {150, 0}, true);
            {
                for(int i=0; i<settings.size(); ++i)
                {
                    if(ui::Selectable(settings[i].c_str(), selected_settings == i))
                        selected_settings = i;
                }
            }
            ui::EndChild();
            ui::SameLine();

            ui::BeginGroup();
            {
                ui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
                {
                    switch(selected_settings)
                    {
                        case 0:
                        {
                            simulated_output::singleton.draw_gui();
                        } break;
                        case 1:
                        {
                            ui::ShowStyleEditor();
                        } break;
                    }
                }
                ui::EndChild();
            }
            ui::EndGroup();
        }
        ui::End();
    }

    previous_pointers = present_pointers;
    present_pointers.clear();

    if (ui::BeginMainMenuBar())
    {
        if (ui::BeginMenu("File"))
        {
            ui::MenuItem("Settings...", NULL, &show_settings);
            ui::EndMenu();
        }
        if (ui::BeginMenu("Tools"))
        {
            if (ui::BeginMenu("ImGui"))
            {
                ui::MenuItem("Metrics/Debugger...", NULL, &show_imgui_metrics);
                ui::MenuItem("Style Editor...", NULL, &show_style_editor);
                ui::MenuItem("Demo...", NULL, &show_imgui_demo);
                ui::MenuItem("About Dear ImGui...", NULL, &show_imgui_about);
                ui::EndMenu();
            }
            ui::MenuItem("About touchrect...", NULL, &show_about);
            ui::EndMenu();
        }
        ui::EndMainMenuBar();
    }
    ui::End();
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
        } break;
        case WM_POINTERACTIVATE:
        {
            result = PA_NOACTIVATE;
        } break;
        case WM_MOUSEACTIVATE:
        {
            if(!present_pointers.empty())
            {
                result = MA_NOACTIVATE;
            }
        } break;
        }
    }
    return result;
}
