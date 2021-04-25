#pragma once
#ifndef FDBG_CONTROLLER_C_THREADS_H_
#define FDBG_CONTROLLER_C_THREADS_H_

#include <fdbg/controller/controller_interface.hpp>
#include <fdbg/model/m_threads.hpp>

struct threads_controller : controller<threads_controller, threads_model>
{
	void register_thread(DWORD id_, HANDLE handle_, std::string name_);
	void unregister_thread(DWORD id_);
	threads_model::thread_entry& current_thread();
	void set_current_thread(DWORD id_);
	decltype(threads_model::threads)& get_threads();
	void update_handles();
};

#endif