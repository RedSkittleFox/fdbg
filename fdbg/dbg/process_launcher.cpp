#include <future>
#include <cstring>

#include <fdbg/filemin.hpp>

#include <fdbg/imgui/imgui.h>
#include <fdbg/nativefiledialog/nfd.h>

#include <fdbg/dbg/process_launcher.hpp>
#include <fdbg/dbg/process.hpp>

process_launcher& process_launcher::instance()
{
	static process_launcher pc;
	return pc;
}

void process_launcher::update()
{
    static std::future<std::string> executable_path;
    static std::future<std::string> env_path;

    if (m_visible)
    {
        ImGui::OpenPopup("Launch Process");
        m_visible = false;
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
        ImGui::InputText("##ep", this->defaults.path.data(), this->defaults.path.size());
        ImGui::SameLine();
        if (ImGui::Button("Browse files..."))
        {
            executable_path = std::async(std::launch::async, get_file_path, std::string("exe"));
        }
        if (executable_path.valid())
        {
            auto str = executable_path.get();
            if (!str.empty())
                strncpy_s(defaults.path.data(), defaults.path.size(), str.c_str(), str.size());
        }

        ImGui::Separator();
        ImGui::TextUnformatted("Launch parameters:");
        ImGui::InputText("##param", this->defaults.params.data(), this->defaults.params.size());

        ImGui::Separator();
        ImGui::TextUnformatted("Environment path:");
        ImGui::InputText("##env", this->defaults.env.data(), this->defaults.env.size());
        ImGui::SameLine();
        if (ImGui::Button("Browse paths..."))
        {
            env_path = std::async(std::launch::async, get_folder_path);
        }
        if (env_path.valid())
        {
            auto str = env_path.get();
            if (!str.empty())
                strncpy_s(defaults.env.data(), defaults.env.size(), str.c_str(), str.size());
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
                process::instance().start(defaults.path.data(), defaults.params.data(), defaults.env.data());

                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();
            ImGui::SetNextItemWidth(80);
            if (ImGui::Button("Cancel"))
            {
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndChild();
        

        ImGui::EndPopup();
    }
}

void process_launcher::pop()
{
	m_visible = true;
}
