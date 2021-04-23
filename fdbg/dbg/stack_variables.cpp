#include <fdbg/imgui/imgui.h>
#include <fdbg/dbg/stack_trace.hpp>
#include <fdbg/dbg/stack_variables.hpp>

stack_variables& stack_variables::instance()
{
    static stack_variables inst;
    return inst;
}

void stack_variables::update()
{
    auto stack = stack_trace::instance().current_stack_entry();

    if (ImGui::Begin("Stack Variables", &m_enabled))
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
