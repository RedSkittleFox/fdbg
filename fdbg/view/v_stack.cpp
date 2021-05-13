#include <fdbg/imgui/imgui.h>
#include <fdbg/view/view_interface.hpp>
#include <fdbg/model/m_stack.hpp>
#include <fdbg/controller/c_stack.hpp>
#include <fdbg/controller/c_source_view.hpp>

struct stack_trace_view : public view<stack_trace_view, stack_trace_model>
{
    stack_trace_view();
	void draw();
};

struct stack_variables_view : public view<stack_variables_view, stack_variables_model>
{
    stack_variables_view();
	void draw();
};

stack_trace_view::stack_trace_view()
{
    vmodel().hideable = true;
    vmodel().name = "Stack Trace";
    vmodel().visible = true;
}

void stack_trace_view::draw()
{
    if (ImGui::Begin("Stack Trace", &(vmodel().visible)))
    {
        if (ImGui::BeginTable("StackTraceTable", 4,
            ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_ScrollY))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_NoHide,
                20, 0);
            ImGui::TableSetupColumn("External",
                ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_NoHide,
                0, 1);
            ImGui::TableSetupColumn("Address",
                ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_NoHide,
                0, 2);
            ImGui::TableSetupColumn("File",
                ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_NoHide,
                0, 3);

            ImGui::TableHeadersRow();

            ImGuiListClipper clipper;
            clipper.Begin(mvc<stack_model>().call_stack.size());
            while (clipper.Step())
            {
                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; ++row_n)
                {
                    auto& entry = mvc<stack_model>().call_stack[row_n];

                    ImGui::TableNextRow(ImGuiTableRowFlags_None, 0);

                    ImGui::TableSetColumnIndex(0);
                    ImGui::PushID(row_n);
                    if (ImGui::Selectable(entry.name.c_str(), mvc<stack_model>().current_stack_entry == row_n, ImGuiSelectableFlags_SpanAllColumns))
                    {
                        auto& smdl = mvc<stack_model>();
                        smdl.current_stack_entry = row_n;
                        mvc<source_view_controller>().set_file(
                            smdl.call_stack[row_n].source_file,
                            smdl.call_stack[row_n].line_number,
                            true);
                    }

                    ImGui::PopID();

                    ImGui::TableSetColumnIndex(1);
                    if (entry.external)
                        ImGui::TextUnformatted("External");

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%x", entry.address);

                    std::string name;
                    if (!entry.external)
                        name = std::string(entry.source_file, entry.source_file.find_last_of('\\') + 1, std::string::npos);

                    ImGui::TableSetColumnIndex(3);
                    ImGui::TextUnformatted(name.c_str());

                }
            }
            clipper.End();

            ImGui::EndTable();
        }
    }
    ImGui::End();
}

stack_variables_view::stack_variables_view()
{
    vmodel().hideable = true;
    vmodel().name = "Stack Variables";
    vmodel().visible = true;
}

void stack_variables_view::draw()
{
    auto stack = mvc<stack_controller>().current_stack_entry();

    if (ImGui::Begin("Stack Variables", &(vmodel().visible)))
    {
        if (ImGui::BeginTable("StackVariablesTable", 2,
            ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_ScrollY))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_NoHide,
                20, 0);

            ImGui::TableHeadersRow();

            ImGuiListClipper clipper;
            clipper.Begin(stack.variables.size());
            while (clipper.Step())
            {
                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; ++row_n)
                {
                    auto& variable = stack.variables[row_n];

                    ImGui::TableNextRow(ImGuiTableRowFlags_None, 0);

                    ImGui::TableSetColumnIndex(0);
                    ImGui::PushID(row_n);
                    if (ImGui::Selectable(variable->name.c_str(), false, ImGuiSelectableFlags_SpanAllColumns));
                    ImGui::PopID();

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("Name: %s Size: %i", variable->type->name.c_str(), variable->type->id);
                }
            }
            clipper.End();

            ImGui::EndTable();
        }
    }
    ImGui::End();
}
