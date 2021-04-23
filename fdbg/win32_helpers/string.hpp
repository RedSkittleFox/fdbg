#ifndef FDBG_WIN32_HELPERS_STRING_H_
#define FDBG_WIN32_HELPERS_STRING_H_
#pragma once

#include <string>

// Convrts wide string to string, dest_len_ should include (char)0x0 in the size.
size_t wstr_to_str(char* dest_, const wchar_t* src_, size_t dest_len_);

std::string to_string_from_wstring(const std::wstring& str_);


#endif