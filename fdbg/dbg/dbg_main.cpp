#include <fdbg/imgui/imgui.h>

void dbg_update()
{
    // Declare Central dockspace
    ImGui::DockSpaceOverViewport();

    static bool show_window = true;
    if (show_window)
    {
        ImGui::Begin("Another Window", &show_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show_window = false;
        ImGui::End();
    }

    if (ImGui::Begin("Master Window"/*, nullptr, ImGuiWindowFlags_MenuBar*/))
    {
        ImGui::TextUnformatted("DockSpace below");

    }
    ImGui::End();


    if (ImGui::Begin("Dockable Window"))
    {
        ImGui::TextUnformatted("Test");
    }
    ImGui::End();
}