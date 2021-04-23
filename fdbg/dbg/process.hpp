#pragma once
#ifndef FDBG_DBG_PROCESS_H_
#define FDBG_DBG_PROCESS_H_

#include <string>
#include <fdbg/win32_helpers/windows.hpp>

class process
{
	DWORD m_process = 0;
	HANDLE m_handle = 0;
	bool m_should_kill_process = false;
public:
	static process& instance();
public:
	void start(const std::string& path_, std::string cmd_ = "", const std::string& env_ = "");
	void attach(DWORD p_);
	void detach();
	DWORD get_process() const noexcept;
	HANDLE handle();
	void handle(HANDLE hndl_);
	bool valid();
	bool should_kill();
};

#endif