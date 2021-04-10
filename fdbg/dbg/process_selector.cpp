#include <vector>
#include <string>
#include <thread>
#include <future>
#include <type_traits>
#include <utility>
#include <array>
#include <algorithm>
#include <numeric>

#include <fdbg/winmin.hpp>
#include <fdbg/imgui/imgui.h>

#include <fdbg/dbg/process_selector.hpp>
#include <fdbg/dbg/process.hpp>

static std::string get_process_name(DWORD pid)
{
    static std::array<char, MAX_PATH> process_name;

    HANDLE hp = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
        false,
        pid);

    if (hp == 0)
        return std::string();

    HMODULE hm;
    DWORD cbn;
    
    if (!EnumProcessModules(hp, &hm, sizeof(hm), &cbn))
        return std::string();

    GetModuleBaseName(hp, hm, process_name.data(), process_name.size() / sizeof(char));
    CloseHandle(hp);

    return std::string(process_name.data());
}

static std::vector<std::pair<DWORD, std::string>> enum_proceseses()
{
    static constexpr size_t enum_count = 2048;
    std::vector<DWORD> data(enum_count);

    DWORD read_size;
    if (!EnumProcesses(data.data(), enum_count, &read_size))
    {
        // Something went wrong. Report this?
        return std::vector<std::pair<DWORD, std::string>>();
    }

    data.resize(read_size < enum_count ? read_size : enum_count);

    {
        std::vector<std::pair<DWORD, std::string>> ret(data.size());
        std::transform(std::begin(data), std::end(data), std::begin(ret),
            [](DWORD pid__) -> std::pair<DWORD, std::string> { return { pid__, get_process_name(pid__) }; });

        std::erase_if(ret, [](const auto& e__)
            {
                return (e__.first == 0 || e__.second == std::string());
            });

        ret.shrink_to_fit();
        std::sort(std::begin(ret), std::end(ret), [](const auto& lhs__, const auto& rhs__) {return lhs__.second < rhs__.second; });

        return ret;
    }
}

process_selector& process_selector::instance()
{
	static process_selector p;
	return p;
}

void process_selector::update()
{
    // Future used to retrieve running processes
    static std::future<std::invoke_result_t<decltype(enum_proceseses)>> enum_process_result;
    if (m_visible)
    {
        ImGui::OpenPopup("Select Process");
        m_visible = false;
        
        enum_process_result = std::async(std::launch::async, enum_proceseses);        
    }

    // Set the proper size even when we resize
    if (ImGui::IsPopupOpen("Select Process"))
    {
        // Center the window
        auto& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), 0, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x - 100, io.DisplaySize.y - 100));
    }

    if (ImGui::BeginPopupModal("Select Process", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar))
    {
        // Used for filtering, stores indexes of filtered items
        static std::vector<size_t> filter_entries;
        static bool filter_name = true; // Filter by name
        static bool filter_pid = false; // Filter by pid
        static std::array<char, MAX_PATH> filter_filter; // Buffer with filter
        static ptrdiff_t currently_selected = -1;
        // Get processes
        if (enum_process_result.valid())
        {
            this->m_processes = enum_process_result.get();
            filter_entries = std::vector<size_t>(m_processes.size());
            std::iota(std::begin(filter_entries), std::end(filter_entries), 0);
        }

        // Menu bar for filters
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Filters"))
            {
                if (ImGui::MenuItem("Filter by Name", nullptr, &filter_name))
                {
                    filter_name = true; filter_pid = false;
                }
                if (ImGui::MenuItem("Filter by PID", nullptr, &filter_pid))
                {
                    filter_name = false; filter_pid = true;
                }
                
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        ImGui::CalcItemWidth();

        // Create child to control window height
        ImGui::BeginChild("ProcessesTable", ImVec2(0, -25), false, ImGuiWindowFlags_AlwaysAutoResize);
        if (ImGui::BeginTable("Processes", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 50.f, 0);
            ImGui::TableSetupColumn("Name", 
                ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 
                400, 1);

            ImGui::TableHeadersRow();

            // Sort our data if sort specs have been changed!
            // We are sorting our filter table by values it points to
            {
                static bool items_need_sort = false;
                ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs();
                if (sorts_specs && sorts_specs->SpecsDirty)
                    items_need_sort = true;
                if (sorts_specs && items_need_sort)
                {
                    // Our column to be sorted is 0
                    if (sorts_specs->Specs->ColumnIndex == 0)
                    {
                        ImGuiSortDirection dir = sorts_specs->Specs->SortDirection;
                        std::sort(std::begin(filter_entries), std::end(filter_entries),
                            [&](const auto& lhs__, const auto& rhs__)
                            {
                                bool res = m_processes[lhs__].first < m_processes[rhs__].first;
                                if (dir == ImGuiSortDirection_Ascending)
                                    return res;
                                return !res;
                            });

                    }
                    // Our column to be sorted is 1
                    else if (sorts_specs->Specs->ColumnIndex == 1)
                    {
                        ImGuiSortDirection dir = sorts_specs->Specs->SortDirection;
                        std::sort(std::begin(filter_entries), std::end(filter_entries),
                            [&](const auto& lhs__, const auto& rhs__)
                            {
                                if (dir == ImGuiSortDirection_Ascending)
                                    return m_processes[lhs__].second < m_processes[rhs__].second;
                                return m_processes[lhs__].second > m_processes[rhs__].second;
                            });

                    }
                    sorts_specs->SpecsDirty = false;
                }
                items_need_sort = false;
            }
            
            // Demonstrate using clipper for large vertical lists
            ImGuiListClipper clipper;
            clipper.Begin(filter_entries.size());
            while (clipper.Step())
            {
                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
                {
                    auto& proc = m_processes[filter_entries[row_n]];

                    // const bool item_is_selected = selection.contains(item->ID);    
                    ImGui::PushID(proc.first);
                    ImGui::TableNextRow(ImGuiTableRowFlags_None, 0);

                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%04d", proc.first);

                    
                    ImGui::TableSetColumnIndex(1);
                    if (ImGui::Selectable(proc.second.c_str(), currently_selected == filter_entries[row_n], ImGuiSelectableFlags_SpanAllColumns))
                    {
                        currently_selected = filter_entries[row_n];
                    }
                    // ImGui::TextUnformatted(proc.second.c_str());

                    ImGui::PopID();
                }
            }

            ImGui::EndTable();
        }
        ImGui::EndChild();

        ImGui::BeginChild("Text Box", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Separator();
        ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - 80 - 80 + 35);
        
        struct callback_input_data
        {
            std::vector<size_t>* filtered_entries;
            const std::vector<std::pair<DWORD, std::string>>* processes;
            bool by_pid;
        };

        callback_input_data data{&filter_entries, &m_processes, filter_pid };

        ImGuiInputTextCallback callback = [](ImGuiInputTextCallbackData* data__) -> int
        {
            callback_input_data* _this = std::bit_cast<callback_input_data*>(data__->UserData);
            _this->filtered_entries->clear();

            bool by_pid = _this->by_pid; // Localize data

            for (size_t i = 0; i < _this->processes->size(); ++i)
            {
                if (by_pid)
                {
                    std::string val = std::to_string(_this->processes->operator[](i).first);
                    if (val.starts_with(data__->Buf))
                        _this->filtered_entries->push_back(i);
                }
                else
                {
                    if (_this->processes->operator[](i).second.starts_with(data__->Buf))
                        _this->filtered_entries->push_back(i);
                }
            }
            return 0;
        };

        if (ImGui::InputText("##label", filter_filter.data(), filter_filter.size(), ImGuiInputTextFlags_CallbackEdit, callback, &data));
        ImGui::SameLine(ImGui::GetWindowWidth() - 120);
        ImGui::SetNextItemWidth(80);
        if (ImGui::Button("Select"))
        {
            if (currently_selected >= 0)
            {
                DWORD current_pid = m_processes[filter_entries[currently_selected]].first;
                process::instance().attach(current_pid);
            }

            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine(ImGui::GetWindowWidth() - 65);
        ImGui::SetNextItemWidth(80);
        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndChild();

        ImGui::EndPopup();
    }
}

void process_selector::pop()
{
    m_visible = true;
}
