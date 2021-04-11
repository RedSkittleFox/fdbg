#include <mutex>
#include <atomic>
#include <string>

#include <fdbg/imgui/imgui.h>
#include <fdbg/winmin.hpp>

#include <fdbg/dbg/menu_bar.hpp>
#include <fdbg/dbg/process_selector.hpp>
#include <fdbg/dbg/process.hpp>
#include <fdbg/dbg/tool_bar.hpp>
#include <fdbg/dbg/output.hpp>
#include <fdbg/dbg/debug_task_queue.hpp>
#include <fdbg/dbg/threads.hpp>
#include <fdbg/dbg/dlls.hpp>
#include <fdbg/dbg/process_launcher.hpp>

// Hack: exclusion of code
static std::mutex& mutex()
{
    static std::mutex mx;
    return mx;
}

// TODO: This doesn't have to be an atomic
static std::atomic_bool db_communication_loop_flag = true;
void dbg_communication_loop_stop()
{
    mutex().lock();
    process::instance().detach();
    db_communication_loop_flag = false;
    mutex().unlock();
}

void dbg_communication_loop()
{
    static DEBUG_EVENT dbe;
    DWORD continue_status = DBG_CONTINUE;

    while (db_communication_loop_flag)
    {
        debug_task_queue::instance().execute();
        if (!process::instance().valid())
            continue;

        WaitForDebugEvent(&dbe, INFINITE);
        mutex().lock();

        switch (dbe.dwDebugEventCode)
        {
        case EXCEPTION_DEBUG_EVENT:
            switch (dbe.u.Exception.ExceptionRecord.ExceptionCode)
            {
            case EXCEPTION_ACCESS_VIOLATION:
                break;
            case EXCEPTION_BREAKPOINT:
                break;
            case EXCEPTION_DATATYPE_MISALIGNMENT:
                break;
            case EXCEPTION_SINGLE_STEP:
                break;
            case DBG_CONTROL_C:
                break;
            default:
                break;
            }
            break;
        case CREATE_THREAD_DEBUG_EVENT:
            threads::instance().register_thread(dbe.dwThreadId, dbe.u.CreateThread.hThread, "");
            output::instance().printl("Debug", std::string("Thread ") + std::to_string(dbe.dwThreadId) + std::string(" created."));
            break;
        case EXIT_THREAD_DEBUG_EVENT:
            threads::instance().unregister_thread(dbe.dwThreadId);
            output::instance().printl("Debug", std::string("Thread ") + std::to_string(dbe.dwThreadId) + std::string(" exited with the code ") +
            std::to_string(dbe.u.ExitThread.dwExitCode) + std::string("."));
            break;
        case CREATE_PROCESS_DEBUG_EVENT:
        {
            LPVOID start_address = std::bit_cast<LPVOID>(dbe.u.CreateProcessInfo.lpStartAddress);
            std::string filename = GetFileNameFromHandle(dbe.u.LoadDll.hFile);
            output::instance().printl("Debug", std::string("Loaded '") + filename + std::string("'."));
            dlls::instance().register_dll(filename, start_address);

            // TODO: Break point?

            break;
        }
        case EXIT_PROCESS_DEBUG_EVENT:
            break;
        case LOAD_DLL_DEBUG_EVENT:
        {
            LPVOID start_address = std::bit_cast<LPVOID>(dbe.u.LoadDll.lpBaseOfDll);
            std::string filename = GetFileNameFromHandle(dbe.u.LoadDll.hFile);
            output::instance().printl("Debug", std::string("Loaded '") + filename + std::string("'."));
            dlls::instance().register_dll(filename, start_address);
            // dbe.u.LoadDll.
            break;
        }
        case UNLOAD_DLL_DEBUG_EVENT:
        {
            LPVOID start_address = dbe.u.UnloadDll.lpBaseOfDll;
            std::string filename = dlls::instance().unregister_dll(start_address);
            output::instance().printl("Debug", std::string("Unloaded '") + filename + std::string("'."));
            break;
        }
        case OUTPUT_DEBUG_STRING_EVENT:
            output::instance().printl("Debug", dbe.u.DebugString.lpDebugStringData);
            break;
        case RIP_EVENT:
            // TODO: Detach debugger
            output::instance().printl("Debug", "Process terminated...");
            break;
        }

        ContinueDebugEvent(dbe.dwProcessId, dbe.dwThreadId, continue_status);
        mutex().unlock();
    }
    int a;
}

void dbg_update()
{
    mutex().lock();
    // Declare Central dockspace
    ImGui::DockSpaceOverViewport();

    menu_bar::instance().update();
    process_selector::instance().update();
    tool_bar::instance().update();
    output::instance().update();
    threads::instance().update();
    process_launcher::instance().update();

    static bool show_window = true;
    if (show_window)
    {
        ImGui::Begin("Another Window", &show_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show_window = false;
        ImGui::End();
    }

    if (ImGui::Begin("Master Window"/*, nullptr, ImGuiWindowFlags_MenuBar*/))
    {
        ImGui::TextUnformatted("DockSpace below");

    }
    ImGui::End();

    static bool gay = true;
    ImGui::ShowDemoWindow(&gay);

    if (ImGui::Begin("Dockable Window"))
    {
        ImGui::TextUnformatted("Test");
    }
    ImGui::End();

    mutex().unlock();
}