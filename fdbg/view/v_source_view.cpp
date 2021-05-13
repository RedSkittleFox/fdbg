#include <fdbg/imgui/imgui.h>
#include <fdbg/view/view_interface.hpp>
#include <fdbg/model/m_source_view.hpp>
#include <fdbg/controller/c_source_view.hpp>

struct source_view_view : public view<source_view_view, source_view_model>
{
	source_view_view();
	void draw();
};

source_view_view::source_view_view()
{
	vmodel().visible = true;
	vmodel().hideable = true;
	vmodel().name = "Source View";
}

void source_view_view::draw()
{	
	if (ImGui::Begin("Source View", &vmodel().visible))
	{
		auto& current = model().current_file;
		if (current.file == nullptr) goto end;

		if (ImGui::BeginTable("SourceViewTable", 2,
			ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_ScrollY))
		{
			ImGui::TableSetupColumn("Break Point", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_NoHide);
			ImGui::TableSetupColumn("Code",
				ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_NoHide, 30);

			ImGuiListClipper clipper;
			// clipper.StartPosY = current_stack_entry.line_number;
			clipper.Begin(current.file->size());
			while (clipper.Step())
			{
				for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; ++row_n)
				{
					auto& line = current.file->operator[](row_n);

					ImGui::TableNextRow(ImGuiTableRowFlags_None, 0);

					ImGui::TableSetColumnIndex(0);

					ImGui::TableSetColumnIndex(1);
					ImGui::PushID(row_n);
					ImGui::Selectable(line.c_str(), row_n == current.line_number);
					ImGui::PopID();
				}
			}

			if (current.center)
			{
				current.center = false;
				// TODO: FIXME
				// ImGui::SetScrollY(clipper.ItemsHeight * (current_stack_entry.line_number - 10));
			};

			clipper.End();

			ImGui::EndTable();
		}
	}
	end:
	ImGui::End();
}
