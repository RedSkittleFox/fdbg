#include <fdbg/winmin.hpp>
#include <fdbg/window/window.hpp>
#include <fdbg/window/graphics.hpp>
#include <fdbg/window/imgui.hpp>

#include <thread>
#include <atomic>

void dbg_update();
void dbg_communication_loop();
void dbg_communication_loop_stop();

INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PSTR lpCmdLine, INT nCmdShow)
{
    auto wnd = create_window(hInstance);
    create_graphics(wnd);
    create_imgui(wnd);

    std::jthread db_thread(dbg_communication_loop);

    while (update_window(wnd))
    {
        update_graphics();
        update_imgui();

        // Main debugger loop
        dbg_update();

        render(wnd);
    }

    dbg_communication_loop_stop();
    
    destroy_imgui();
    destroy_graphics(wnd);
    destroy_window(wnd);

    return 0;
}