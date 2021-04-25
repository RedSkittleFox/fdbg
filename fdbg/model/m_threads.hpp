#pragma once
#ifndef FDBG_MODEL_M_THREADS_H_
#define FDBG_MODEL_M_THREADS_H_

#include <fdbg/win32_helpers/windows.hpp>
#include <fdbg/model/model_interface.hpp>

struct threads_model : public view_model<threads_model>
{
	struct thread_entry
	{
		DWORD id;
		HANDLE handle;
		std::string name;
		bool debug_enabled;
	};

	std::vector<thread_entry> threads;
	size_t current_thread = 0;
};

#endif