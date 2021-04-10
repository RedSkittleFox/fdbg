#pragma once
#ifndef FDBG_DBG_PROCESS_H_
#define FDBG_DBG_PROCESS_H_

#include <fdbg/winmin.hpp>

class process
{
	DWORD m_process = 0;

public:
	static process& instance();
public:
	void attach(DWORD p_);
	void detach();
	DWORD get_process() const noexcept;
	bool valid();
};

#endif