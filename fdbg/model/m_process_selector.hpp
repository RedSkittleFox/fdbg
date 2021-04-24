#pragma once
#ifndef RDBG_MODEL_M_PROCESS_SELECTOR_H_
#define RDBG_MODEL_M_PROCESS_SELECTOR_H_

#include <vector>
#include <string>
#include <future>
#include <fdbg/win32_helpers/windows.hpp>

#include <fdbg/model/model_interface.hpp>

struct process_selector_model : public view_model<process_selector_model>
{
	std::vector<std::pair<DWORD, std::string>> processes;
	std::future<std::invoke_result_t<decltype(enum_proceseses)>> enum_process_result;
	bool popped = false;

	struct
	{
		// Used for filtering, stores indexes of filtered items
		std::vector<size_t> filter_entries;
		bool filter_name = true; // Filter by name
		bool filter_pid = false; // Filter by pid
		std::string filter_text; // Buffer with filter
		ptrdiff_t currently_selected = -1;
		bool items_need_sort = false;
	} filtering;
};

#endif