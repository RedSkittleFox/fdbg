#pragma once
#ifndef FDBG_DBG_PROCESS_SELECTOR_H_
#define FDBG_DBG_PROCESS_SELECTOR_H_

#include <vector>
#include <string>
#include <utility>

#include <fdbg/win32_helpers/windows.hpp>

class process_selector
{
	bool m_visible;
	std::vector<std::pair<DWORD, std::string>> m_processes;
public:
	static process_selector& instance();

public:
	void update();
	void pop();
};

#endif
