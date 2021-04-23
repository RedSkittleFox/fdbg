#include <fdbg/imgui/imgui.h>
#include <fdbg/win32_helpers/windows.hpp>

#include <fdbg/dbg/tool_bar.hpp>
#include <fdbg/dbg/break_points.hpp>
#include <fdbg/dbg/process.hpp>

tool_bar& tool_bar::instance()
{
	static tool_bar tb;
	return tb;
}

void tool_bar::update()
{
	ImGuiWindowFlags window_flags = 0
		| ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoScrollbar
		;

	ImGui::Begin("TOOLBAR", NULL, window_flags);

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

	ImGui::End();
}