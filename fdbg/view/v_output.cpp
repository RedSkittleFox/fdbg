#include <fdbg/imgui/imgui.h>
#include <fdbg/view/view_interface.hpp>
#include <fdbg/model/m_output.hpp>

class output_view : public view<output_view, output_model>
{
public:
    virtual void draw() override final;
};

void output_view::draw()
{
    if (ImGui::Begin("Output", &vmodel().visible))
    {
        // Output mode selection
        ImGui::Text("Show output from:");
        ImGui::SameLine();

        // Output mode combo
        ImGui::PushID("OutputMode");
        static size_t currently_selected = 0;
        auto& current = model().output_buffers[currently_selected];
        if (ImGui::BeginCombo("", current.first.c_str(), ImGuiComboFlags_HeightRegular))
        {
            for (size_t i = 0; i < model().output_buffers.size(); ++i)
            {
                if (ImGui::Selectable(model().output_buffers[i].first.c_str(), i == currently_selected))
                {
                    currently_selected = i;
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopID();
        ImGui::Separator();
        
        // Output mode text
        ImGui::BeginChild("OutputText");
        ImGui::InputTextMultiline("##source", current.second.data(), current.second.size(), ImVec2(ImGui::GetWindowContentRegionWidth(), 0.f),
            ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_ReadOnly);
        ImGui::EndChild();
    }
    ImGui::End();
}
