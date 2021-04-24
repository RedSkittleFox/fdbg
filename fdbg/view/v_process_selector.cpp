#include <fdbg/imgui/imgui.h>
#include <fdbg/imgui/imgui_stdlib.h>

#include <fdbg/view/view_interface.hpp>
#include <fdbg/model/m_process_selector.hpp>
#include <fdbg/controller/c_process_selector.hpp>
#include <fdbg/dbg/debug_task_queue.hpp>
#include <fdbg/dbg/process.hpp>

struct process_selector_view : public view<process_selector_view, process_selector_model>
{
public:
    process_selector_view();
public:
	void draw();

public: // UI
    void menu_bar();
    void process_table();
    void sort_process_table();
};

process_selector_view::process_selector_view()
{
    vmodel().hideable = false;
    vmodel().visible = false;
}

void process_selector_view::draw()
{
    if (model().view.visible && model().popped == false)
    {
        ImGui::OpenPopup("Select Process");
        model().popped = true;

        mvc<process_selector_controller>().update_process_list();
    }

    // Set the proper size even when we resize
    if (ImGui::IsPopupOpen("Select Process"))
    {
        // Center the window
        auto& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), 0, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x - 100, io.DisplaySize.y - 100));
    }

    if (ImGui::BeginPopupModal("Select Process", nullptr, 
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar))
    {
        menu_bar();

        process_table();

        ImGui::BeginChild("Text Box", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Separator();
        ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - 80 - 80 + 35);

        struct callback_input_data
        {
            std::vector<size_t>* filtered_entries;
            const std::vector<std::pair<DWORD, std::string>>* processes;
            bool by_pid;
        };

        callback_input_data data{ &model().filtering.filter_entries, &model().processes, model().filtering.filter_pid };

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

        if (ImGui::InputText("##label", &model().filtering.filter_text, ImGuiInputTextFlags_CallbackEdit, callback, &data));
        ImGui::SameLine(ImGui::GetWindowWidth() - 120);
        ImGui::SetNextItemWidth(80);
        if (ImGui::Button("Select"))
        {
            if (model().filtering.currently_selected >= 0)
            {
                DWORD current_pid = model().processes[model().filtering.filter_entries[model().filtering.currently_selected]].first;
                debug_task_queue::instance().push([=]() { process::instance().attach(current_pid); });
            }

            ImGui::CloseCurrentPopup();
            model().popped = false;
            vmodel().visible = false;
        }

        ImGui::SameLine(ImGui::GetWindowWidth() - 65);
        ImGui::SetNextItemWidth(80);
        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
            model().popped = false;
            vmodel().visible = false;
        }
        ImGui::EndChild();

        ImGui::EndPopup();
    }
}

void process_selector_view::menu_bar()
{
    // Menu bar for filters
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Filters"))
        {
            if (ImGui::MenuItem("Filter by Name", nullptr, &model().filtering.filter_name))
            {
                model().filtering.filter_name = true; 
                model().filtering.filter_pid = false;
            }
            if (ImGui::MenuItem("Filter by PID", nullptr, &model().filtering.filter_pid))
            {
                model().filtering.filter_name = false; 
                model().filtering.filter_pid = true;
            }

            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void process_selector_view::process_table()
{
    // Create child to control window height
    ImGui::BeginChild("ProcessesTable", ImVec2(0, -25), false, ImGuiWindowFlags_AlwaysAutoResize);
    if (ImGui::BeginTable("Processes", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable))
    {
        // Setup tables
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 50.f, 0);
        ImGui::TableSetupColumn("Name",
            ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide,
            400, 1);

        // Draw Row Descriptions
        ImGui::TableHeadersRow();

        sort_process_table();

        // Draw table
        ImGuiListClipper clipper;
        clipper.Begin(model().filtering.filter_entries.size());
        while (clipper.Step())
        {
            for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
            {
                auto& proc = model().processes[model().filtering.filter_entries[row_n]];

                // const bool item_is_selected = selection.contains(item->ID);    
                ImGui::PushID(proc.first);
                ImGui::TableNextRow(ImGuiTableRowFlags_None, 0);

                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%04d", proc.first);


                ImGui::TableSetColumnIndex(1);
                if (ImGui::Selectable(proc.second.c_str(), model().filtering.currently_selected == model().filtering.filter_entries[row_n], 
                    ImGuiSelectableFlags_SpanAllColumns))
                {
                    model().filtering.currently_selected = model().filtering.filter_entries[row_n];
                }
                // ImGui::TextUnformatted(proc.second.c_str());

                ImGui::PopID();
            }
        }

        ImGui::EndTable();
    }
    ImGui::EndChild();
}

void process_selector_view::sort_process_table()
{
    // Sort our data if sort specs have been changed!
    // We are sorting our filter table by values it points to
    ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs();
    if (sorts_specs && sorts_specs->SpecsDirty)
        model().filtering.items_need_sort = true;
    if (sorts_specs && model().filtering.items_need_sort)
    {
        // Our column to be sorted is 0
        if (sorts_specs->Specs->ColumnIndex == 0)
        {
            ImGuiSortDirection dir = sorts_specs->Specs->SortDirection;
            std::sort(std::begin(model().filtering.filter_entries), std::end(model().filtering.filter_entries),
                [&, this](const auto& lhs__, const auto& rhs__)
                {
                    bool res = model().processes[lhs__].first < model().processes[rhs__].first;
                    if (dir == ImGuiSortDirection_Ascending)
                        return res;
                    return !res;
                });

        }
        // Our column to be sorted is 1
        else if (sorts_specs->Specs->ColumnIndex == 1)
        {
            ImGuiSortDirection dir = sorts_specs->Specs->SortDirection;
            std::sort(std::begin(model().filtering.filter_entries), std::end(model().filtering.filter_entries),
                [&, this](const auto& lhs__, const auto& rhs__)
                {
                    if (dir == ImGuiSortDirection_Ascending)
                        return model().processes[lhs__].second < model().processes[rhs__].second;
                    return model().processes[lhs__].second > model().processes[rhs__].second;
                });

        }
        sorts_specs->SpecsDirty = false;
    }
    model().filtering.items_need_sort = false;
}
