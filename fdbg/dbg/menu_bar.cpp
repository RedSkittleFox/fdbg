#include <fdbg/winmin.hpp>
#include <fdbg/imgui/imgui.h>

#include <fdbg/dbg/menu_bar.hpp>
#include <fdbg/dbg/process.hpp>
#include <fdbg/dbg/process_selector.hpp>
#include <fdbg/dbg/process_launcher.hpp>

menu_bar& menu_bar::instance()
{
    static menu_bar b;
    return b;
}

void menu_bar::update()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Start New Process"))
            {
                process_launcher::instance().pop();
            }
            if (ImGui::MenuItem("Attach Process"))
            {
                process_selector::instance().pop();
            }
            if (ImGui::MenuItem("Detach Process"))
            {
                process::instance().detach();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "ALT+F4"))
            {
                PostQuitMessage(0);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X")) {}
            if (ImGui::MenuItem("Copy", "CTRL+C")) {}
            if (ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X")) {}
            if (ImGui::MenuItem("Copy", "CTRL+C")) {}
            if (ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Debug"))
        {
            ImGui::Checkbox("Break on first instruction.", &config.debug.break_on_first_instruction);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}