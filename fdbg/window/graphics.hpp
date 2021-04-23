#pragma once
#ifndef FDBG_WINDOW_GRAPHICS_H_
#define FDBG_WINDOW_GRAPHICS_H_

#include <fdbg/win32_helpers/windows.hpp>

void create_graphics(HWND);
void update_graphics();
void destroy_graphics(HWND);

#endif