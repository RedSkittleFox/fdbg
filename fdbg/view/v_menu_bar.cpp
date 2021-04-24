///////////////////////////////////////////////////////////////////////////////
// Author:		Marcin Poloczek (RedSkittleFox)
// Contact:		RedSkittleFox@gmail.com
// Copyright:	Refer to project's license.
// Purpose:		Implementation of Main Menu View
// 

#include <fdbg/imgui/imgui.h>
#include <fdbg/model/m_menu_bar.hpp>
#include <fdbg/view/view_interface.hpp>

///////////////////////////////////////////////////////////////////////////////
// Symbol:  menu_bar_view
// Purpose: Main Menu Bar View
//
class menu_bar_view : public view<menu_bar_view, menu_bar_model>
{
public:
    menu_bar_view();
    menu_bar_view(const menu_bar_view&) = default;
    menu_bar_view(menu_bar_view&&) noexcept = default;
    menu_bar_view& operator=(const menu_bar_view&) = default;
    menu_bar_view& operator=(menu_bar_view&&) noexcept = default;
    virtual ~menu_bar_view() noexcept = default;

public:
    virtual void draw() override final;

private:
    // Menus
    void menu_file();
    void menu_edit();
    void menu_view();
    void menu_debug();
};

///////////////////////////////////////////////////////////////////////////////
// menu_bar_view implementations
//

menu_bar_view::menu_bar_view()
{
    vmodel().visible = true;
    vmodel().hideable = false;
    vmodel().name = "Menu Bar";
}

void menu_bar_view::draw()
{
    if (ImGui::BeginMainMenuBar())
    {
        menu_file();
        menu_edit();
        menu_view();
        menu_debug();
        
        ImGui::EndMainMenuBar();
    }
}

///////////////////////////////////////////////////////////////////////////////
// Menus Imlementations
//

void menu_bar_view::menu_file()
{
    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("Start New Process"))
        {
            // process_launcher::instance().pop();
        }
        if (ImGui::MenuItem("Attach Process"))
        {
            // process_selector::instance().pop();
        }
        if (ImGui::MenuItem("Detach Process"))
        {
            // process::instance().detach();
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Exit", "ALT+F4"))
        {
            // PostQuitMessage(0);
        }
        ImGui::EndMenu();
    }
}

void menu_bar_view::menu_edit()
{
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
}

void menu_bar_view::menu_view()
{
    if (ImGui::BeginMenu("View"))
    {
        // Show all gui windows
        for (auto& gui_element : view_manager::instance().views())
        {
            if (gui_element->vmodel().hideable)
            {
                if(ImGui::MenuItem(gui_element->vmodel().name.c_str()))
                    gui_element->vmodel().visible = !gui_element->vmodel().visible;
            }
        }
        ImGui::EndMenu();
    }
}

void menu_bar_view::menu_debug()
{
    if (ImGui::BeginMenu("Debug"))
    {
        ImGui::Checkbox("Break on first instruction.", &model().debug.break_on_first_instruction);
        ImGui::EndMenu();
    }
}