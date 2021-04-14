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
#include <fdbg/dbg/break_points.hpp>
#include <fdbg/dbg/registers.hpp>

DWORD64 get_start_address(HANDLE hProcess)
{
    SYMBOL_INFO* pSymbol;
    IMAGEHLP_SYMBOL64 sym = {};
    sym.SizeOfStruct = sizeof(sym);

    pSymbol = (SYMBOL_INFO*)new BYTE[sizeof(SYMBOL_INFO) + MAX_SYM_NAME];
    pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    pSymbol->MaxNameLen = MAX_SYM_NAME;
    bool val = SymFromName(hProcess, "WinMainCRTStartup", pSymbol);

    // Store address, before deleting pointer  
    DWORD64 dwAddress = pSymbol->Address;

    delete[](BYTE*)pSymbol; // Valid syntax!

    return dwAddress;
}

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

// TODO: Refactor me
void dbg_communication_loop()
{
    static DEBUG_EVENT dbe;
    DWORD continue_status = DBG_CONTINUE;
    static bool hit_once = false;

    while (db_communication_loop_flag)
    {
        // Execute remote task
        mutex().lock();
        debug_task_queue::instance().execute();
        
        // Skip if process isn't valid
        if (!process::instance().valid()) // FIXME: This causes spin lock
            goto end;

        if (break_points::instance().triggered()) goto end;

        mutex().unlock();
        
        if(!WaitForDebugEvent(&dbe, INFINITE)) goto end;
        mutex().lock();

        break_points::instance().set_debug_identifiers(dbe.dwProcessId, dbe.dwThreadId);

        // Set current thread
        if (dbe.dwProcessId == process::instance().get_process())
        {
            threads::instance().set_current_thread(dbe.dwThreadId);
            // threads::instance().update_handles();
        }

        switch (dbe.dwDebugEventCode)
        {
        case EXCEPTION_DEBUG_EVENT:
        {
            switch (dbe.u.Exception.ExceptionRecord.ExceptionCode)
            {
            case EXCEPTION_ACCESS_VIOLATION:
                break;
            case EXCEPTION_BREAKPOINT:
            {
                
                if (hit_once == false)
                {
                    hit_once = true;
                    ContinueDebugEvent(dbe.dwProcessId, dbe.dwThreadId, continue_status);
                    goto end;
                }
                // Try handling stepping breakpoint

                if (dbe.dwProcessId == process::instance().get_process())
                {
                    // Check if we are a step break point and if yes do sth about it
                    auto curr = threads::instance().current_thread().id;
                    auto c2 = dbe.dwThreadId;

                    break_points::instance().revert_break_point(dbe.u.Exception.ExceptionRecord.ExceptionAddress);
                }
                break;
            }
            case EXCEPTION_DATATYPE_MISALIGNMENT:
                break;
            case EXCEPTION_SINGLE_STEP:
            {
                if (dbe.dwProcessId == process::instance().get_process())
                {
                    break_points::instance().rever_trap_break_points();

                    if (threads::instance().current_thread().debug_enabled != true)
                    {
                        break_points::instance().create_trap_break_point();
                        ContinueDebugEvent(dbe.dwProcessId, dbe.dwThreadId, continue_status);
                        goto end;
                    }
                }
                break;
            }
            case DBG_CONTROL_C:
                break;
            case EXCEPTION_PRIV_INSTRUCTION:
            {
                auto p = dbe.u.Exception.ExceptionRecord.ExceptionAddress;
                ContinueDebugEvent(dbe.dwProcessId, dbe.dwThreadId, continue_status);
                goto end;
                break;
            }
            default:
                ContinueDebugEvent(dbe.dwProcessId, dbe.dwThreadId, continue_status);
                goto end;
                break;
            }
            break_points::instance().trigger();
            registers::instance().update_context();
            goto end; // Don't call continue debug
        }
        case CREATE_THREAD_DEBUG_EVENT:
            if(dbe.dwProcessId == process::instance().get_process())
                threads::instance().register_thread(dbe.dwThreadId, dbe.u.CreateThread.hThread, "");
            output::instance().printl("Debug", std::string("Thread ") + std::to_string(dbe.dwThreadId) + std::string(" created."));
            break;
        case EXIT_THREAD_DEBUG_EVENT:
            if(dbe.dwProcessId == process::instance().get_process())
                threads::instance().unregister_thread(dbe.dwThreadId);
            output::instance().printl("Debug", std::string("Thread ") + std::to_string(dbe.dwThreadId) + std::string(" exited with the code ") +
            std::to_string(dbe.u.ExitThread.dwExitCode) + std::string("."));
            break;
        case CREATE_PROCESS_DEBUG_EVENT:
        {
            hit_once = false;
            std::string filename = GetFileNameFromHandle(dbe.u.LoadDll.hFile);
            output::instance().printl("Debug", std::string("Loaded '") + filename + std::string("'."));
            if (dbe.dwProcessId == process::instance().get_process())
            {
                threads::instance().register_thread(dbe.dwThreadId, OpenThread(THREAD_ALL_ACCESS, false, dbe.dwThreadId), "<main thread>");

                dlls::instance().register_dll(filename,
                    dbe.u.CreateProcessInfo.lpStartAddress,
                    dbe.u.CreateProcessInfo.dwDebugInfoFileOffset);

                // TODO: Create a breakpoint on the first instruciton
                if (menu_bar::instance().config.debug.break_on_first_instruction)
                {
                    void* add = (void*)get_start_address(process::instance().handle());
                    void* b = (void*)((DWORD64)dbe.u.CreateProcessInfo.lpBaseOfImage + (DWORD64)add);
                    void* other = dbe.u.CreateProcessInfo.lpStartAddress;
                    break_points::instance().create_break_point(other);
                }
            }

            break;
        }
        case EXIT_PROCESS_DEBUG_EVENT:
            if(dbe.dwProcessId == process::instance().get_process())
                process::instance().detach();
            output::instance().printl("Debug", std::string("Process has exited with code '") + std::to_string(dbe.u.ExitProcess.dwExitCode) + std::string("."));

            break;
        case LOAD_DLL_DEBUG_EVENT:
        {
            LPVOID start_address = std::bit_cast<LPVOID>(dbe.u.LoadDll.lpBaseOfDll);
            std::string filename = GetFileNameFromHandle(dbe.u.LoadDll.hFile);
            output::instance().printl("Debug", std::string("Loaded '") + filename + std::string("'."));
            if(dbe.dwProcessId == process::instance().get_process())
                dlls::instance().register_dll(filename, start_address, dbe.u.LoadDll.dwDebugInfoFileOffset);
            // dbe.u.LoadDll.
            break;
        }
        case UNLOAD_DLL_DEBUG_EVENT:
        {
            LPVOID start_address = dbe.u.UnloadDll.lpBaseOfDll;
            std::string filename = dlls::instance().unregister_dll(start_address);
            if(dbe.dwProcessId == process::instance().get_process())
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
        end:
        mutex().unlock();
    }
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
    registers::instance().update();

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