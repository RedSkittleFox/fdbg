#include <fdbg/imgui/imgui.h>

#include <fdbg/dbg/threads.hpp>

threads& threads::instance()
{
	static threads t;
	return t;
}

void threads::update()
{
	if (ImGui::Begin("Threads", &m_enabled))
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
			clipper.Begin(m_threads.size());
			while (clipper.Step())
			{
				
				for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
				{
					auto& thread = m_threads[row_n];

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
					ImGui::Selectable(thread.name.c_str(), m_current_thread == row_n, ImGuiSelectableFlags_SpanAllColumns);

				}
			}
			clipper.End();

			ImGui::EndTable();
		}
	}
	ImGui::End();
}

void threads::register_thread(DWORD id_, HANDLE handle_, std::string name_)
{
	m_threads.push_back({id_, handle_, name_, true});
}

void threads::unregister_thread(DWORD id_)
{
	std::erase_if(m_threads, [=](const auto& e__)
		{
			return e__.id == id_;
		});
}

threads::thread_entry_t& threads::current_thread()
{
	// TODO: Do a check
	return m_threads[m_current_thread];
}

void threads::set_current_thread(DWORD id_)
{
	auto res = std::find_if(std::begin(m_threads), std::end(m_threads), [=](const auto& e__)
		{
			return e__.id == id_;
		});

	m_current_thread = std::distance(std::begin(m_threads), res) - 1;
}

void threads::update_handles()
{
	for (auto& t : m_threads)
	{
		t.handle = OpenThread(THREAD_ALL_ACCESS, false, t.id);
	}
}

decltype(threads::m_threads)& threads::get_threads()
{
	return m_threads;
}
