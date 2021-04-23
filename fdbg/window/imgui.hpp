#pragma once
#ifndef FDBG_WINDOW_IMGUI_H_
#define FDBG_WINDOW_IMGUI_H_

#include <fdbg/win32_helpers/windows.hpp>

extern void create_imgui(HWND wnd);
extern void update_imgui();
extern void render(HWND wnd);
extern void destroy_imgui();

#endif