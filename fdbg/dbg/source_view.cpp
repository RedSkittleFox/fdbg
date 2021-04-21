#include <fdbg/imgui/imgui.h>
#include <fdbg/dbg/source_view.hpp>
#include <fdbg/dbg/stack_trace.hpp>

source_view& source_view::instance()
{
    static source_view inst;
    return inst;
}

static std::vector<std::string> source_file_loader(const std::string& file_)
{
	if (file_.empty()) return std::vector<std::string>();

	std::ifstream file;
	file.open(file_);
	if (file.bad()) return std::vector<std::string>();

	std::vector<std::string> data;
	std::string line;
	while (std::getline(file, line))
	{
		data.push_back(line);
	}

	return data;
}

void source_view::update()
{
	static struct
	{
		std::future<std::vector<std::string>> file_data;
		std::string current_name;
		void* current_address = nullptr;
		bool center = true;
	} s;

	if (ImGui::Begin("Source View", &m_enabled))
	{
		auto current_stack_entry = stack_trace::instance().current_stack_entry();
		auto current = m_files.find(current_stack_entry.source_file);
		
		if (current_stack_entry.address != s.current_address)
		{
			s.current_address = current_stack_entry.address;
			s.center = true;
		}

		if (current == std::end(m_files))
		{
			// Check if our previous file is ready
			if(s.file_data.valid())
			{
				m_files[s.current_name] = s.file_data.get();
			}

			if (s.current_name != current_stack_entry.name)
			{
				s.current_name = current_stack_entry.source_file;
				s.file_data = std::async(std::launch::async, source_file_loader, s.current_name);
			};
		}
		else if (ImGui::BeginTable("SourceViewTable", 2,
			ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_ScrollY))
		{
			ImGui::TableSetupColumn("Break Point", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_NoHide);
			ImGui::TableSetupColumn("Code",
				ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_NoHide, 30);

			ImGuiListClipper clipper;
			// clipper.StartPosY = current_stack_entry.line_number;
			clipper.Begin(current->second.size());
			while (clipper.Step())
			{
				for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; ++row_n)
				{
					auto& line = current->second[row_n];

					ImGui::TableNextRow(ImGuiTableRowFlags_None, 0);

					ImGui::TableSetColumnIndex(0);

					ImGui::TableSetColumnIndex(1);
					ImGui::PushID(row_n);
					ImGui::Selectable(current->second[row_n].c_str(), row_n == current_stack_entry.line_number);
					ImGui::PopID();
				}
			}

			if (s.center)
			{
				s.center = false;
				ImGui::SetScrollY(clipper.ItemsHeight * (current_stack_entry.line_number - 10));
			};

			clipper.End();

			ImGui::EndTable();
		}
	}
	ImGui::End();
}
