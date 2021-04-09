#pragma once
#ifndef FDBG_WINDOW_WINDOW_H_
#define FDBG_WINDOW_WINDOW_H_

#include <cstdint>
#include <fdbg/winmin.hpp>

HWND create_window(HINSTANCE hinstance);
bool update_window(HWND);
void destroy_window(HWND hwnd);

#endif