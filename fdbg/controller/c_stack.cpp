#include <fdbg/controller/c_stack.hpp>
#include <fdbg/controller/c_output.hpp>

#include <fdbg/dbg/process.hpp>

static BOOL WINAPI enum_symbols_callback(SYMBOL_INFO* sym_info__,
    ULONG size__,
    PVOID context__)
{
    auto ptr = imagehlp_symbol_manager::instance().varaible(sym_info__->Index, sym_info__->ModBase);
    if (ptr == nullptr) return true;

    std::vector<imagehlp_symbol_variable*>* variables = std::bit_cast<std::vector<imagehlp_symbol_variable*>*>(context__);
    variables->push_back(ptr);

    return true;
};

void stack_controller::break_point()
{
    // Clear the call stack
    model().call_stack.clear();

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
        bool res = StackWalk64(
            IMAGE_FILE_MACHINE_AMD64, 
            process::instance().handle(), 
            dbg_thread(), 
            &stack,
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
        model().call_stack.push_back(
            stack_model::call_stack_entry
            {
                .name = std::string(symbol.Name),
                .address = std::bit_cast<void*>(symbol.Address),
                .external = external == false ? true : false, // We are asigning values > 1 to this, is this safe?,
                .line_number = line_number,
                .source_file = filename,
                .variables = variables
            }
        );

        mvc<output_controller>().printl("Debug", symbol.Name);

    } while (stack.AddrReturn.Offset != 0);

    model().current_stack_entry = 0;
}

stack_model::call_stack_entry stack_controller::current_stack_entry()
{
    auto stack = stack_model::call_stack_entry{};
    if (model().call_stack.size() != 0 && model().call_stack.size() > model().current_stack_entry)
        stack = model().call_stack[model().current_stack_entry];

    return stack;
}
