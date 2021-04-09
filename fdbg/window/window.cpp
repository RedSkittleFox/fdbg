#include <fdbg/window/window.hpp>

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wp, lp))
        return true;

    switch (msg)
    {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, msg, wp, lp);
    }
}

HWND create_window(HINSTANCE hinstance)
{
    // Register the window class.
    const char CLASS_NAME[] = "Sample Window Class";

    WNDCLASS wc = { };

    wc.lpfnWndProc = window_proc;
    wc.hInstance = hinstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window.

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        "Learn to Program Windows",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window    
        NULL,       // Menu
        hinstance,  // Instance handle
        NULL        // Additional application data
    );

    if (hwnd == NULL)
    {
        return static_cast<HWND>(0);
    }

    ShowWindow(hwnd, true);
    return hwnd;
}

bool update_window(HWND wnd)
{
    static MSG msg;
    if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) == false) return true;
    
    TranslateMessage(&msg);
    DispatchMessage(&msg);

    if (msg.message == WM_QUIT)
        return false;

    return true;
}

void destroy_window(HWND hwnd)
{
    DestroyWindow(hwnd);
}