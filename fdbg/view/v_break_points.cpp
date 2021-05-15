#include <fdbg/imgui/imgui.h>
#include <fdbg/model/model_interface.hpp>
#include <fdbg/view/view_interface.hpp>
#include <fdbg/model/m_break_points.hpp>

struct break_points_view : public view<break_points_view, break_points_model>
{
	break_points_view();
	virtual void draw() override final;
};

break_points_view::break_points_view()
{
	vmodel().hideable = true;
	vmodel().name = "Breakpoints";
	vmodel().visible = true;
}

void break_points_view::draw()
{
    if (ImGui::Begin("Break Points", &(vmodel().visible)))
    {
        if (ImGui::BeginTable("BreakPointsTable", 3,
            ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_ScrollY))
        {
            ImGui::TableSetupColumn("File", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_NoHide,
                20, 0);

            ImGui::TableSetupColumn("Line", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_NoHide,
                20, 1);

            ImGui::TableSetupColumn("Condition",
                ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_NoHide,
                0, 2);

            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableHeadersRow();

            auto mdl = model().break_points;
            for (size_t i = {}; i < mdl.size(); ++i)
            {
                if (!mdl[i].single_hit)
                {
                    ImGui::TableNextRow(ImGuiTableRowFlags_None, 0);

                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(mdl[i].source.c_str());
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%i", mdl[i].line);
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("");
                }
            }

            ImGui::EndTable();
        }
    }
    ImGui::End();
}
