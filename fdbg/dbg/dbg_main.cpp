#include <mutex>
#include <atomic>

#include <fdbg/imgui/imgui.h>
#include <fdbg/winmin.hpp>

#include <fdbg/dbg/menu_bar.hpp>
#include <fdbg/dbg/process_selector.hpp>
#include <fdbg/dbg/process.hpp>
#include <fdbg/dbg/tool_bar.hpp>
#include <fdbg/dbg/output.hpp>

// Hack: exclusion of code
static std::mutex& mutex()
{
    static std::mutex mx;
    return mx;
}

static std::atomic_bool db_communication_loop_flag = true;
void dbg_communication_loop_stop()
{
    db_communication_loop_flag = false;
}

void dbg_communication_loop()
{
    static DEBUG_EVENT dbe;
    DWORD continue_status = DBG_CONTINUE;

    while (db_communication_loop_flag)
    {
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
            break;
        case CREATE_PROCESS_DEBUG_EVENT:
            break;
        case EXIT_THREAD_DEBUG_EVENT:
            break;
        case EXIT_PROCESS_DEBUG_EVENT:
            break;
        case LOAD_DLL_DEBUG_EVENT:
            output::instance().printl("Debug", (const char*)dbe.u.LoadDll.lpImageName);
            break;
        case UNLOAD_DLL_DEBUG_EVENT:
            break;
        case OUTPUT_DEBUG_STRING_EVENT:
            output::instance().printl("Debug", dbe.u.DebugString.lpDebugStringData);
            break;
        case RIP_EVENT:
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