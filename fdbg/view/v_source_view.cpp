#include <fdbg/imgui/imgui.h>
#include <fdbg/view/view_interface.hpp>
#include <fdbg/model/m_source_view.hpp>
#include <fdbg/controller/c_source_view.hpp>
#include <fdbg/model/m_break_points.hpp>
#include <fdbg/controller/c_break_points.hpp>
#include <fdbg/dbg/debug_task_queue.hpp>

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
				for (size_t row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; ++row_n)
				{
					auto& line = current.file->operator[](row_n);

					ImGui::TableNextRow(ImGuiTableRowFlags_None, 0);

					ImGui::TableSetColumnIndex(0);
					// This is SUPER SLOW
					// TODO: Implement faster solution
					{
						bool selected = false;
						typename break_points_model::break_point* pbp = nullptr;

						auto& bpm = mvc<break_points_model>();
						for (auto& bp : bpm.break_points)
						{
							if (bp.line - 1 == row_n
								&& bp.source == model().current_file.file_name)
							{
								selected = true;
								pbp = &bp;
								goto loop_end;
							}
						}
					loop_end:

						ImGui::PushID(row_n << 2);
						ImGui::Checkbox("", &selected);
						ImGui::PopID();

						bool in_break = mvc<break_points_model>().in_break;
						debug_task_queue::instance().push(
							[=]
							{
								if (in_break)
								{
									if (selected == true && pbp == nullptr)
										mvc<break_points_controller>().create_break_point(model().current_file.file_name, row_n + 1);
									else if (selected == false && pbp != nullptr)
										mvc<break_points_controller>().revert_break_point(model().current_file.file_name, row_n + 1);
								}
							}
							);
						
					}

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
