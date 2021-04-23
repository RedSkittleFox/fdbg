#include <fdbg/win32_helpers/windows.hpp>
#include <fdbg/win32_helpers/string.hpp>
#include <fdbg/win32_helpers/dbg_help.hpp>

#include <fdbg/imgui/imgui.h>

#include <fdbg/dbg/stack_trace.hpp>
#include <fdbg/dbg/process.hpp>
#include <fdbg/dbg/threads.hpp>
#include <fdbg/dbg/output.hpp>
#include <fdbg/dbg/dlls.hpp>

stack_trace& stack_trace::instance()
{
    static stack_trace inst;
    return inst;
}

// Wrapper to convert arguments
// TODO: Should this be moved to winmin.hpp?
static BOOL __stdcall process_memory_reader(
    HANDLE phandle,
    DWORD64 base,
    PVOID buffer,
    DWORD size,
    LPDWORD bytes_read
)
{
    return ReadProcessMemory(phandle, std::bit_cast<LPCVOID>(base), buffer, size, std::bit_cast<SIZE_T*>(bytes_read));
}

void stack_trace::update()
{
    if (ImGui::Begin("Stack Trace", &m_enabled))
    {
        if (ImGui::BeginTable("StackTraceTable", 4,
            ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchSame| ImGuiTableFlags_ScrollY))
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
            clipper.Begin(m_call_stack.size());
            while (clipper.Step())
            {
                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; ++row_n)
                {
                    auto& entry = m_call_stack[row_n];

                    ImGui::TableNextRow(ImGuiTableRowFlags_None, 0);

                    ImGui::TableSetColumnIndex(0);
                    ImGui::PushID(row_n);
                    if (ImGui::Selectable(entry.name.c_str(), m_current_stack_entry == row_n, ImGuiSelectableFlags_SpanAllColumns))
                    {
                        m_current_stack_entry = row_n;
                    }

                    ImGui::PopID();

                    ImGui::TableSetColumnIndex(1);
                    if(entry.external)
                        ImGui::TextUnformatted("External");

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%x", entry.address);

                    std::string name;
                    if(!entry.external)
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

BOOL WINAPI enum_symbols_callback(SYMBOL_INFO* sym_info__,
    ULONG size__,
    PVOID context__)
{    
    auto ptr = imagehlp_symbol_manager::instance().varaible(sym_info__->Index, sym_info__->ModBase);
    if (ptr == nullptr) return true;

    std::vector<imagehlp_symbol_variable*>* variables = std::bit_cast<std::vector<imagehlp_symbol_variable*>*>(context__);
    variables->push_back(ptr);

    return true;
};

void stack_trace::update_data()
{
    // Clear the call stack
    m_call_stack.clear();

    CONTEXT context = current_context();

    // Initialize 'stack' with some required stuff.
    STACKFRAME64 stack = { };
    // Using flat memory model
    stack.AddrPC.Mode = AddrModeFlat;
    stack.AddrFrame.Mode = AddrModeFlat;
    stack.AddrStack.Mode = AddrModeFlat;

    stack.AddrPC.Offset = context.Rip; // RIP - Instruction Pointer
    stack.AddrFrame.Offset = context.Rbp; // RBP - Stack base, RDX is used for argument passing
    stack.AddrStack.Offset = context.Rsp; // RSP - Stack Pointer
    
    // Push elements onto call stack
    do
    {
        bool res = StackWalk64(IMAGE_FILE_MACHINE_AMD64, process::instance().handle(), threads::instance().current_thread().handle, &stack,
            &context, nullptr, SymFunctionTableAccess64,
            SymGetModuleBase64, 0);

        if (!res) break;

        imagehlp_module64 module;
        SymGetModuleInfo64(process::instance().handle(), stack.AddrPC.Offset, &module);
        
        DWORD64 displacement;
        imagehlp_symbol64 symbol;
        
        SymGetSymFromAddr64(process::instance().handle(), stack.AddrPC.Offset, &displacement, &symbol);

        if (symbol.Address == 0) continue;

        imagehlp_line64 line;
        
        DWORD disp;
        bool external = SymGetLineFromAddr64(process::instance().handle(), stack.AddrPC.Offset, &disp, &line);
        
        std::string filename;
        size_t line_number = 0;
        if (external != false)
        {
            filename = std::string(line.FileName);
            line_number = line.LineNumber;
        }
        
        std::vector<imagehlp_symbol_variable*> variables;
        {
            res = SymSetScopeFromAddr(process::instance().handle(), stack.AddrPC.Offset);
            res = SymEnumSymbols(process::instance().handle(),
                0,
                "",
                enum_symbols_callback,
                &variables);
            auto error = GetLastError();
        }

        // Push entry onto callstack
        m_call_stack.push_back(
            call_stack_entry
            { 
                .name = std::string(symbol.Name), 
                .address = std::bit_cast<void*>(symbol.Address),
                .external = external == false ? true : false, // We are asigning values > 1 to this, is this safe?,
                .line_number = line_number,
                .source_file = filename,
                .variables = variables
            }
        );

        output::instance().printl("Debug", symbol.Name);

    } while (stack.AddrReturn.Offset != 0);
}

stack_trace::call_stack_entry stack_trace::current_stack_entry()
{
    if (m_current_stack_entry < m_call_stack.size())
        return m_call_stack[m_current_stack_entry];

    return call_stack_entry();
}
