#include <fdbg/winmin.hpp>
#include <fdbg/window/window.hpp>
#include <fdbg/window/graphics.hpp>
#include <fdbg/window/imgui.hpp>

void dbg_update();

INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PSTR lpCmdLine, INT nCmdShow)
{
    auto wnd = create_window(hInstance);
    create_graphics(wnd);
    create_imgui(wnd);

    while (update_window(wnd))
    {
        update_graphics();
        update_imgui();

        // Main debugger loop
        dbg_update();

        render(wnd);
    }

    destroy_imgui();
    destroy_graphics(wnd);
    destroy_window(wnd);

    return 0;
}