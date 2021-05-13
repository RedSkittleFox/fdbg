#include <fdbg/imgui/imgui.h>
#include <fdbg/view/view_interface.hpp>
#include <fdbg/model/m_stack.hpp>
#include <fdbg/controller/c_stack.hpp>
#include <fdbg/controller/c_source_view.hpp>
#include <fdbg/win32_helpers/dbg_help.hpp>

struct stack_trace_view : public view<stack_trace_view, stack_trace_model>
{
    stack_trace_view();
	void draw();
};

struct stack_variables_view : public view<stack_variables_view, stack_variables_model>
{
    stack_variables_view();
	void draw();
    void draw_variable(imagehlp_symbol_variable* var_, size_t id_);
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
        if (ImGui::BeginTable("StackVariablesTable", 3,
            ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_ScrollY))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_NoHide,
                20, 0);

            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_NoHide,
                20, 0);

            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_NoHide,
                20, 0);

            ImGui::TableHeadersRow();

            for (size_t i{ 0llu }; i < stack.variables.size(); ++i)
            {
                draw_variable(stack.variables[i], i << 4);
            }

            ImGui::EndTable();
        }
    }
    ImGui::End();
}

void stack_variables_view::draw_variable(imagehlp_symbol_variable* var_, size_t id_)
{
    ImGui::TableNextRow(ImGuiTableRowFlags_None, 0);

    using st = imagehlp_symbol_type;
        
    // TODO: Enumeration type?
    if (std::holds_alternative<typename st::fundamental>(var_->type->u))
    {
        ImGui::TableSetColumnIndex(0);
        ImGui::PushID(id_);
        if (ImGui::Selectable(var_->name.c_str(), false, ImGuiSelectableFlags_SpanAllColumns));
        ImGui::PopID();

        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s", var_->type->name.c_str());
    }
    else if (std::holds_alternative<typename st::array>(var_->type->u))
    {

    }
    else if (std::holds_alternative<typename st::pointer>(var_->type->u))
    {
        ImGui::TableSetColumnIndex(0);
        ImGui::PushID(id_);
        if (ImGui::Selectable(var_->name.c_str(), false, ImGuiSelectableFlags_SpanAllColumns));
        ImGui::PopID();

        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s", var_->type->name.c_str());
    }
    else if (std::holds_alternative<typename st::reference>(var_->type->u))
    {
        ImGui::TableSetColumnIndex(0);
        ImGui::PushID(id_);
        if (ImGui::Selectable(var_->name.c_str(), false, ImGuiSelectableFlags_SpanAllColumns));
        ImGui::PopID();

        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s", var_->type->name.c_str());
    }
    else if (std::holds_alternative<typename st::type_def>(var_->type->u))
    {
        ImGui::TableSetColumnIndex(0);
        ImGui::PushID(id_);
        if (ImGui::Selectable(var_->name.c_str(), false, ImGuiSelectableFlags_SpanAllColumns));
        ImGui::PopID();

        ImGui::TableSetColumnIndex(1);
        
        // TODO: Can this be something else than fundamental?
        typename st::fundamental& base = std::get<typename st::fundamental>(var_->type->u);
        
        std::string name = imagehlp_get_fundamental_name(base.type, var_->type->size);
        ImGui::Text("%s (%s)", var_->type->name.c_str(), name.c_str());
    }
    else if (std::holds_alternative<typename st::user_defined>(var_->type->u))
    {
        ImGui::TableSetColumnIndex(1);

        // TODO: Can this be something else than fundamental?
        typename st::user_defined& base = std::get<typename st::user_defined>(var_->type->u);
        std::string name = "<unknown>";
        if (base.kind == imagehlp_symbol_type_udt_kind::CLASS) name = "class";
        else if (base.kind == imagehlp_symbol_type_udt_kind::STRUCT) name = "struct";
        else if (base.kind == imagehlp_symbol_type_udt_kind::UNION) name = "union";
        else if (base.kind == imagehlp_symbol_type_udt_kind::INTERFACE) name = "interface";
        
        ImGui::Text("%s (%s)", var_->type->name.c_str(), name.c_str());

        ImGui::TableSetColumnIndex(0);
        bool open = ImGui::TreeNodeEx(var_->name.c_str(), ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_SpanFullWidth);

        if (open)
        {
            size_t i = id_;
            for (auto& e : base.variables)
            {
                ++i;
                draw_variable(e, i);
            }
            ImGui::TreePop();
        }
    }

    
}
