#include <fdbg/imgui/imgui.h>

#include <fdbg/view/view_interface.hpp>
#include <fdbg/model/m_threads.hpp>

struct threads_view : public view<threads_view, threads_model>
{
	threads_view();
	void draw();
};

threads_view::threads_view()
{
	vmodel().hideable = true;
	vmodel().visible = true;
	vmodel().name = "Threads";
}

void threads_view::draw()
{
	if (ImGui::Begin("Threads", &(vmodel().visible)))
	{
		if (ImGui::BeginTable("ThreadsTable", 4,
			ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable))
		{
			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableSetupColumn("Enable Debugging", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide,
				20, 0);
			ImGui::TableSetupColumn("ID",
				ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide,
				0, 1);
			ImGui::TableSetupColumn("Handle",
				ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide,
				0, 2);
			ImGui::TableSetupColumn("Description",
				ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide,
				0, 3);

			ImGui::TableHeadersRow();

			ImGuiListClipper clipper;
			clipper.Begin(model().threads.size());
			while (clipper.Step())
			{

				for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
				{
					auto& thread = model().threads[row_n];

					ImGui::TableNextRow(ImGuiTableRowFlags_None, 0);

					ImGui::TableSetColumnIndex(0);
					ImGui::PushID(row_n);
					ImGui::Checkbox("", &thread.debug_enabled);
					ImGui::PopID();

					ImGui::TableSetColumnIndex(1);
					ImGui::Text("%04d", thread.id);

					ImGui::TableSetColumnIndex(2);
					ImGui::Text("%04d", thread.handle);

					ImGui::TableSetColumnIndex(3);
					ImGui::Selectable(thread.name.c_str(), model().current_thread == row_n, ImGuiSelectableFlags_SpanAllColumns);

				}
			}
			clipper.End();

			ImGui::EndTable();
		}
	}
	ImGui::End();
}
