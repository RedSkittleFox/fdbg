#include <fdbg/imgui/imgui.h>
#include <fdbg/winmin.hpp>

#include <fdbg/dbg/tool_bar.hpp>

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

	ImGui::Button("Continue");
	ImGui::SameLine();
	ImGui::Button("Break");
	ImGui::SameLine();
	ImGui::Button("Step Over");
	ImGui::SameLine();
	ImGui::Button("Step In");
	ImGui::SameLine();
	ImGui::Button("Step Out");

	ImGui::End();
}