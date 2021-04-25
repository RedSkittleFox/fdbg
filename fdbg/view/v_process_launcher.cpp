#include <fdbg/imgui/imgui.h>
#include <fdbg/imgui/imgui_stdlib.h>
#include <fdbg/win32_helpers/windows.hpp>
#include <fdbg/win32_helpers/filesystem.hpp>

#include <fdbg/view/view_interface.hpp>
#include <fdbg/model/m_process_launcher.hpp>
#include <fdbg/dbg/process.hpp>

struct process_launcher_view : public view<process_launcher_view, process_launcher_model>
{
    process_launcher_view();
	void draw();
};

process_launcher_view::process_launcher_view()
{
    vmodel().visible = false;
    vmodel().hideable = false;
    vmodel().name = "Process Launcher";
}

void process_launcher_view::draw()
{
    if (vmodel().visible)
    {
        ImGui::OpenPopup("Launch Process");
        model().popped = false;
    }

    // Set the proper size even when we resize
    if (ImGui::IsPopupOpen("Launch Process"))
    {
        // Center the window
        auto& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), 0, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x - 200, io.DisplaySize.y - 200));
    }

    if (ImGui::BeginPopupModal("Launch Process", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
    {
        ImGui::TextUnformatted("Executable's path:");
        ImGui::InputText("##ep", &model().config.path, model().config.path.size());
        ImGui::SameLine();
        if (ImGui::Button("Browse files..."))
        {
            model().executable_path = std::async(std::launch::async, get_file_path, std::string("exe"));
        }
        if (model().executable_path.valid())
        {
            model().config.path = model().executable_path.get();
        }

        ImGui::Separator();
        ImGui::TextUnformatted("Launch parameters:");
        ImGui::InputText("##param", &model().config.params, model().config.params.size());

        ImGui::Separator();
        ImGui::TextUnformatted("Environment path:");
        ImGui::InputText("##env", &model().config.env, model().config.env.size());
        ImGui::SameLine();
        if (ImGui::Button("Browse paths..."))
        {
            model().env_path = std::async(std::launch::async, get_folder_path);
        }
        if (model().env_path.valid())
        {
            auto str = model().env_path.get();
            if (!str.empty())
                strncpy_s(model().config.env.data(), model().config.env.size(), str.c_str(), str.size());
        }

        if (ImGui::BeginChild("FreeSpace"))
        {
            ImGui::BeginChild("Padding0", ImVec2(0, ImGui::GetWindowHeight() - 25));
            ImGui::EndChild();

            ImGui::BeginChild("Padding1", ImVec2(ImGui::GetWindowWidth() - 135, 0));
            ImGui::EndChild();

            ImGui::SameLine();
            ImGui::SetNextItemWidth(80);
            if (ImGui::Button("Launch"))
            {
                process::instance().start(
                    model().config.path.data(), 
                    model().config.params.data(), 
                    model().config.env.data());

                vmodel().visible = false;
                model().popped = false;

                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();
            ImGui::SetNextItemWidth(80);
            if (ImGui::Button("Cancel"))
            {
                vmodel().visible = false;
                model().popped = false;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndChild();


        ImGui::EndPopup();
    }
}
