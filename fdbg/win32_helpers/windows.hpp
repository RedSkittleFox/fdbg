#pragma once
#ifndef FDBG_WIN32_HELPERS_WINDOWS_H_
#define FDBG_WIN32_HELPERS_WINDOWS_H_

#define _WIN32_WINNT 0x0601
#include <sdkddkver.h>

#define WIN32_LEAN_AND_MEAN
#define NOGDICAPMASKS
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOSYSCOMMANDS
#define NORASTEROPS
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOKERNEL
#define NONLS
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
// #define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define NORPC
#define NOPROXYSTUB
#define NOIMAGE
#define NOTAPE

#define STRICT

#include <Windows.h>
#include <psapi.h>
#include <strsafe.h>

#include <wrl/client.h>
#include <DbgHelp.h>

using Microsoft::WRL::ComPtr;

#include <exception>
#include <array>
#include <string>
#include <vector>
#include <utility>

#define WIN_ASSERT(X, MSG) if(##X == 0) throw std::exception(##MSG##);

std::string get_process_name(DWORD pid_);
std::vector<std::pair<DWORD, std::string>> enum_proceseses();

// Wrapper to convert arguments
static BOOL __stdcall process_memory_reader(
    HANDLE phandle,
    DWORD64 base,
    PVOID buffer,
    DWORD size,
    LPDWORD bytes_read
);

#endif
