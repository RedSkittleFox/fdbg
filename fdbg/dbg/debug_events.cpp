#include <fdbg/model/m_menu_bar.hpp>

#include <fdbg/dbg/debug_events.hpp>
#include <fdbg/controller/c_threads.hpp>
#include <fdbg/controller/c_break_points.hpp>
#include <fdbg/dbg/process.hpp>
#include <fdbg/controller/c_output.hpp>
#include <fdbg/dbg/dlls.hpp>
#include <fdbg/dbg/debug_task_queue.hpp>
#include <fdbg/win32_helpers/filesystem.hpp>

// After connecting to the debugee a dummy breakpoint is triggered. 
// We want to ignore it.
static bool s_hit_once = false;

bool exception_debug_event(const DEBUG_EVENT& dbe_)
{
    switch (dbe_.u.Exception.ExceptionRecord.ExceptionCode)
    {
    case EXCEPTION_BREAKPOINT:
    {
        if (s_hit_once == false)
        {
            // Enable debugging of DbgHelp
            DWORD options = ::SymGetOptions();
            // options |= SYMOPT_DEBUG | SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME | SYMOPT_LOAD_LINES;
            ::SymSetOptions(options);

            bool ress = SymInitialize(process::instance().handle(), nullptr, true);
            auto errr = GetLastError();
            if (ress == false)
            {
                mvc<output_controller>().printl("Debug", "Failed to initialize symbols.");
            }

            s_hit_once = true;
            return true;
        }

        // Revert single hit breakpoint  
        mvc<break_points_controller>().revert_break_point(dbe_.u.Exception.ExceptionRecord.ExceptionAddress);
        break;
    }
    case EXCEPTION_SINGLE_STEP:
    {
        // Clear all trap breakpoints
        mvc<break_points_controller>().rever_trap_break_points();

        // If debugging current thread is dissabled, 
        // don't enter debug break mode
        if (mvc<threads_controller>().current_thread().debug_enabled != true)
        {
            mvc<break_points_controller>().create_trap_break_point();
            return true;
        }

        break;
    }
    }

    mvc<break_points_controller>().trigger();
	return false;
}

bool create_thread_debug_event(const DEBUG_EVENT& dbe_)
{
    // Register created thread
    mvc<threads_controller>().register_thread(dbe_.dwThreadId, dbe_.u.CreateThread.hThread, "");
    
    // Output debug message
    // TODO: Use <format> or something sprintf_s
    mvc<output_controller>()
        .printl("Debug", 
            std::string("Thread ") + std::to_string(dbe_.dwThreadId) + std::string(" created.")
        );
    
    return true;
}

bool exit_thread_debug_event(const DEBUG_EVENT& dbe_)
{
    // Unregister thread
    mvc<threads_controller>().unregister_thread(dbe_.dwThreadId);
        
    // Output debug message
    mvc<output_controller>()
        .printl("Debug", std::string("Thread ") + std::to_string(dbe_.dwThreadId) + std::string(" exited with the code ") +
        std::to_string(dbe_.u.ExitThread.dwExitCode) + std::string("."));
    
    return true;
}

bool create_process_debug_event(const DEBUG_EVENT& dbe_)
{
    // Set hit once flag to false
    s_hit_once = false;

    // TODO: Clear registers, threads, breakpoints etc.
    process::instance().handle(dbe_.u.CreateProcessInfo.hProcess);
    
    // get executable's file name and print debug message
    std::string filename = get_file_name_from_handle(dbe_.u.LoadDll.hFile);
    mvc<output_controller>().printl("Debug", std::string("Loaded '") + filename + std::string("'."));

    // Register main thread. CREATE_THREAD_DEBUG_EVENT is not called for it.
    mvc<threads_controller>().register_thread(dbe_.dwThreadId, OpenThread(THREAD_ALL_ACCESS, false, dbe_.dwThreadId), "<main thread>");

    // Register dll
    dlls::instance().register_dll(filename,
        dbe_.u.CreateProcessInfo.lpStartAddress,
        dbe_.u.CreateProcessInfo.hFile,
        dbe_.u.CreateProcessInfo.dwDebugInfoFileOffset);

    // Break on entry point
    // TODO: Find a better way to store global config
    if (model_manager::instance().model<menu_bar_model>().debug.break_on_first_instruction)
    {
        mvc<break_points_controller>().create_break_point(dbe_.u.CreateProcessInfo.lpStartAddress);
    }

    return true;
}

bool exit_process_debug_event(const DEBUG_EVENT& dbe_)
{
    process::instance().detach();        
    mvc<output_controller>().printl("Debug", std::string("Process has exited with code '") + std::to_string(dbe_.u.ExitProcess.dwExitCode) + std::string("."));

    // Unregister thread

    // TODO: Clear breakpoints, threads etc.

	return true;
}

bool load_dll_debug_event(const DEBUG_EVENT& dbe_)
{
    // Output debug message
    std::string filename = get_file_name_from_handle(dbe_.u.LoadDll.hFile);
    mvc<output_controller>().printl("Debug", std::string("Loaded '") + filename + std::string("'."));

    // Get base address of dll.
    // It is used to identify dlls.
    void* start_address = std::bit_cast<void*>(dbe_.u.LoadDll.lpBaseOfDll);

    // Register loaded dll.
    dlls::instance().register_dll(filename, start_address, 
        dbe_.u.LoadDll.hFile,
        dbe_.u.LoadDll.dwDebugInfoFileOffset);
        
    return true;
}

bool unload_dll_debug_event(const DEBUG_EVENT& dbe_)
{
    // Locate dll by base address and unregister it
    void* start_address = std::bit_cast<void*>(dbe_.u.UnloadDll.lpBaseOfDll);
    std::string filename = dlls::instance().unregister_dll(start_address);

    // Output debug message
    mvc<output_controller>().printl("Debug", std::string("Unloaded '") + filename + std::string("'."));

	return true;
}

bool output_debug_string_debug_event(const DEBUG_EVENT& dbe_)
{
    // TODO:
    mvc<output_controller>().printl("Debug", "DEBUG MESSAGE SENT");
    return true;
}

bool rip_debug_event(const DEBUG_EVENT& dbe_)
{
    // TODO: Detach debugger
    mvc<output_controller>().printl("Debug", "Process terminated...");
    process::instance().detach();
    // TODO: Clear everything
	return true;
}
