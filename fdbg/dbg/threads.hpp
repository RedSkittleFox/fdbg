#pragma once
#ifndef FDBG_DBG_THREADS_H_
#define FDBG_DBG_THREADS_H_

#include <fdbg/winmin.hpp>
#include <vector>
#include <string>

class threads
{
public:
	typedef struct
	{
		DWORD id;
		HANDLE handle;
		std::string name;
		bool debug_enabled;
	} thread_entry_t;

private:
	std::vector<thread_entry_t> m_threads;
	bool m_enabled = true;
	size_t m_current_thread = 1000;
public:
	static threads& instance();

public:
	void update();

public:
	void register_thread(DWORD id_, HANDLE handle_, std::string name_);
	void unregister_thread(DWORD id_);
};

#endif