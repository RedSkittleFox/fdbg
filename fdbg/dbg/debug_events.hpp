// Author:		Marcin Poloczek (RedSkittleFox)
// Contact:		RedSkittleFox@gmail.com
// Copyright:	Refer to project's license.
// Purpose:		Provide debug event handlers.
#pragma once
#ifndef FDBG_DBG_DEBUG_EVENTS_H_
#define FDBG_DBG_DEBUG_EVENTS_H_

#include <fdbg/winmin.hpp>

// If a funciton returns true then debugging should continue.
// Suspend program's execution otherwise.

bool exception_debug_event(const DEBUG_EVENT& dbe_);
bool create_thread_debug_event(const DEBUG_EVENT& dbe_);
bool exit_thread_debug_event(const DEBUG_EVENT& dbe_);
bool create_process_debug_event(const DEBUG_EVENT& dbe_);
bool exit_process_debug_event(const DEBUG_EVENT& dbe_);
bool load_dll_debug_event(const DEBUG_EVENT& dbe_);
bool unload_dll_debug_event(const DEBUG_EVENT& dbe_);
bool output_debug_string_debug_event(const DEBUG_EVENT& dbe_);
bool rip_debug_event(const DEBUG_EVENT& dbe_);

#endif