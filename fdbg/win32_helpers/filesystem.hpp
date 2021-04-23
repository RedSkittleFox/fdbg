#ifndef FDBG_WIN32_HELPERS_FILESYSTEM_H_
#define FDBG_WIN32_HELPERS_FILESYSTEM_H_
#pragma once

#include <fdbg/win32_helpers/windows.hpp>
#include <string>

std::string get_file_path(const std::string& filter_);
std::string get_folder_path();

// Modified https://docs.microsoft.com/en-us/windows/win32/memory/obtaining-a-file-name-from-a-file-handle
std::string get_file_name_from_handle(HANDLE file_);

#endif