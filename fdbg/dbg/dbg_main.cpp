#include <mutex>
#include <atomic>
#include <string>

#include <fdbg/imgui/imgui.h>
#include <fdbg/win32_helpers/windows.hpp>

#include <fdbg/dbg/menu_bar.hpp>
#include <fdbg/dbg/process_selector.hpp>
#include <fdbg/dbg/process.hpp>
#include <fdbg/dbg/tool_bar.hpp>
#include <fdbg/dbg/output.hpp>
#include <fdbg/dbg/debug_task_queue.hpp>
#include <fdbg/dbg/threads.hpp>
#include <fdbg/dbg/dlls.hpp>
#include <fdbg/dbg/process_launcher.hpp>
#include <fdbg/dbg/break_points.hpp>
#include <fdbg/dbg/registers.hpp>
#include <fdbg/dbg/debug_events.hpp>
#include <fdbg/dbg/stack_trace.hpp>
#include <fdbg/dbg/source_view.hpp>
#include <fdbg/dbg/stack_variables.hpp>

// Hack: exclusion of code

// TODO: This doesn't have to be an atomic
static std::mutex s_thread_mx;
static std::atomic_bool db_communication_loop_flag = true;
void dbg_communication_loop_stop()
{
    std::lock_guard<std::mutex> lg(s_thread_mx);
    process::instance().detach();
    db_communication_loop_flag = false;
}

// TODO: Refactor me
void dbg_communication_loop()
{
    static DEBUG_EVENT dbe;
    DWORD continue_status = DBG_CONTINUE;
    static bool hit_once = false;

    while (db_communication_loop_flag)
    {
        {
            std::lock_guard<std::mutex> lg(s_thread_mx);

            // Execute task queue for this thread
            debug_task_queue::instance().execute();

            // If process isn't valid. Don't try to debug
            if (!process::instance().valid()) continue;

            // If breakpoint is triggered. Don't debug
            if (break_points::instance().triggered()) continue;
        }
        
        // Wait for debug events
        if(!WaitForDebugEvent(&dbe, INFINITE)) continue;
        
        {
            std::lock_guard<std::mutex> lg(s_thread_mx);

            // break_points::instance().set_debug_identifiers(dbe.dwProcessId, dbe.dwThreadId);
            threads::instance().set_current_thread(dbe.dwThreadId);

            // Continue debugging if true
            static bool s_continue = true;

            switch (dbe.dwDebugEventCode)
            {
            case EXCEPTION_DEBUG_EVENT:
            {
                s_continue = exception_debug_event(dbe);
                break;
            }
            case CREATE_THREAD_DEBUG_EVENT:
            {
                s_continue = create_thread_debug_event(dbe);
                break;
            }
            case EXIT_THREAD_DEBUG_EVENT:
            {
                s_continue = exit_thread_debug_event(dbe);
                break;
            }
            case CREATE_PROCESS_DEBUG_EVENT:
            {
                s_continue = create_process_debug_event(dbe);
                break;
            }
            case EXIT_PROCESS_DEBUG_EVENT:
                s_continue = exit_process_debug_event(dbe);
                break;
            case LOAD_DLL_DEBUG_EVENT:
            {
                s_continue = load_dll_debug_event(dbe);
                break;
            }
            case UNLOAD_DLL_DEBUG_EVENT:
            {
                s_continue = unload_dll_debug_event(dbe);
                break;
            }
            case OUTPUT_DEBUG_STRING_EVENT:
                s_continue = output_debug_string_debug_event(dbe);
                break;
            case RIP_EVENT:
                s_continue = rip_debug_event(dbe);
                break;
            }

            if(s_continue) 
                ContinueDebugEvent(dbe.dwProcessId, dbe.dwThreadId, continue_status);
        }
    }
}

void dbg_update()
{
    std::lock_guard<std::mutex> lg(s_thread_mx);
    // Declare Central dockspace
    ImGui::DockSpaceOverViewport();

    menu_bar::instance().update();
    process_selector::instance().update();
    tool_bar::instance().update();
    output::instance().update();
    threads::instance().update();
    process_launcher::instance().update();
    registers::instance().update();
    stack_trace::instance().update();
    source_view::instance().update();
    stack_variables::instance().update();

    static bool demo = true;
    ImGui::ShowDemoWindow(&demo);

}