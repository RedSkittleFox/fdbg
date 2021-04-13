#pragma once
#ifndef FDBG_DBG_BREAK_POINTS_H_
#define FDBG_DBG_BREAK_POINTS_H_

#include <vector>
#include <string>

class break_points
{
public:
	// Inserted by user
	struct break_point
	{
		bool enabled;
		std::string source;
		size_t line;

		std::string dll;
		void* address;
		std::byte replaced_instruction;
	};
	
private:
	bool m_break = false;
	std::vector<break_point> m_break_points;

	DWORD m_proc_id;
	DWORD m_thread_id;

public:
	static break_points& instance();

public:
	void trigger();
	bool triggered();
	void continue_debug();
	void create_break_point(void* address_);
	void revert_break_point(void* address_);
	// Set trap flag on every thread
	void create_trap_break_point();
	// Remove trap flag from every thread
	void rever_trap_break_points();
	void set_debug_identifiers(DWORD proc_, DWORD thread_);
};

#endif