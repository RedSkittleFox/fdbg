#include <fdbg/imgui/imgui.h>

#include <fdbg/view/view_interface.hpp>
#include <fdbg/model/m_tool_bar.hpp>
#include <fdbg/controller/c_tool_bar.hpp>


struct tool_bar_view : public view<tool_bar_view, tool_bar_model>
{
	tool_bar_view();
	void draw();
};

tool_bar_view::tool_bar_view()
{
	vmodel().visible = true;
	vmodel().hideable = true;
	vmodel().name = "Tool Bar";
}

void tool_bar_view::draw()
{
	ImGuiWindowFlags window_flags = 0
		| ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoScrollbar;

	if (ImGui::Begin("TOOLBAR", NULL, window_flags))
	{
		if (ImGui::Button("Continue"))
		{
			mvc<tool_bar_controller>().continue_execution();
		}

		ImGui::SameLine();
		if (ImGui::Button("Break"))
		{
			mvc<tool_bar_controller>().break_execution();
		}
		ImGui::SameLine();

		if (ImGui::Button("Step Over"))
		{
			mvc<tool_bar_controller>().step_over();
		}

		ImGui::SameLine();
		ImGui::Button("Step In");
		ImGui::SameLine();
		ImGui::Button("Step Out");
	}

	ImGui::End();
}
