#include <algorithm>

#include <fdbg/imgui/imgui.h>

#include <fdbg/dbg/output.hpp>

output& output::instance()
{
	static output o;
	return o;
}

output::output()
{
    this->register_output_buffer("Debug");
    this->register_output_buffer("General");
}

void output::update()
{
    ImGui::Begin("Output", &m_visible);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
    ImGui::Text("Show output from:");
    ImGui::SameLine();
    ImGui::PushID("OutputMode");
    static size_t currently_selected = 0;
    auto& current = m_output_buffers[currently_selected];
    if (ImGui::BeginCombo("", current.first.c_str(), ImGuiComboFlags_HeightRegular))
    {
        for(size_t i = 0; i < m_output_buffers.size(); ++i)
        {
            if (ImGui::Selectable(m_output_buffers[i].first.c_str(), i == currently_selected))
            {
                currently_selected = i;
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopID();
    ImGui::Separator();
    ImGui::BeginChild("OutputText");
    
    ImGui::InputTextMultiline("##source", current.second.data(), current.second.size(), ImVec2(ImGui::GetWindowContentRegionWidth(), 0.f),
        ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_ReadOnly);
    ImGui::EndChild();
    ImGui::End();
}

void output::register_output_buffer(const std::string& name_)
{
    m_output_buffers.push_back({name_, std::string("")});
}

void output::print(const std::string& sink_, const std::string& output_)
{
    auto res = std::find_if(std::begin(m_output_buffers), std::end(m_output_buffers), [&](const auto& e__) -> bool { return e__.first == sink_; });
    if (res == std::end(m_output_buffers))
    {
        this->printl("General", std::string("Unknown sink: ") + sink_);
        return;
    }
    res->second.append(output_);
}

void output::printl(const std::string& sink_, const std::string& output_)
{
    auto res = std::find_if(std::begin(m_output_buffers), std::end(m_output_buffers), [&](const auto& e__) -> bool { return e__.first == sink_; });
    if (res == std::end(m_output_buffers))
    {
        this->printl("General", std::string("Unknown sink: ") + sink_);
        return;
    }
    res->second.push_back('\n');
    res->second.append(output_);
}

void output::clear(const std::string& sink_)
{
    auto res = std::find_if(std::begin(m_output_buffers), std::end(m_output_buffers), [&](const auto& e__) -> bool { return e__.first == sink_; });
    if (res == std::end(m_output_buffers))
    {
        this->printl("General", std::string("Unknown sink: ") + sink_);
        return;
    }
    res->second.clear();
}

void output::clear()
{
    for (auto& s : m_output_buffers)
        s.second.clear();
}
