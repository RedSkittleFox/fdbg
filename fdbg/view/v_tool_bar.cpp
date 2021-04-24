#include <fdbg/imgui/imgui.h>
#include <fdbg/win32_helpers/windows.hpp>

#include <fdbg/view/view_interface.hpp>
#include <fdbg/model/m_tool_bar.hpp>

#include <fdbg/dbg/break_points.hpp>
#include <fdbg/dbg/process.hpp>

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
			break_points::instance().continue_debug();
		}

		ImGui::SameLine();
		if (ImGui::Button("Break"))
		{
			break_points::instance().debug_break();
		}
		ImGui::SameLine();

		if (ImGui::Button("Step Over"))
		{
			break_points::instance().create_trap_break_point();
			break_points::instance().continue_debug();
		}

		ImGui::SameLine();
		ImGui::Button("Step In");
		ImGui::SameLine();
		ImGui::Button("Step Out");
	}

	ImGui::End();
}