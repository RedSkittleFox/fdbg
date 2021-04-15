// Author:		Marcin Poloczek (RedSkittleFox)
// Contact:		RedSkittleFox@gmail.com
// Copyright:	Refer to project's license.
// Purpose:		Provide interface for managing break points.
#pragma once
#ifndef FDBG_DBG_BREAK_POINTS_H_
#define FDBG_DBG_BREAK_POINTS_H_

#include <vector>
#include <string>

class break_points
{
public:
	struct break_point
	{
		bool enabled;			// Breakpoint is enabled
		std::string source;		// Source file
		size_t line;			// Source line

		// TODO:
		// bool single_hit;

		std::string dll;		// Dll file
		void* address;			// Address (global)

		// Replaced instruction data
		std::uint8_t replaced_instruction;
	};
	
private:
	bool m_break = false;
	std::vector<break_point> m_break_points;

public:
	static break_points& instance();

public:
	// Used by debug loop to notify break_points that
	// a break point happened. Don't use anywhere else!
	void trigger();

	// Returns true if debugee is suspended. 
	bool triggered();

	// Mark exception as handled. Resume program's execution.
	void continue_debug();

	// Create breakpoint at given address.
	void create_break_point(void* address_);

	// Revert breakpoint to 
	void revert_break_point(void* address_);
	
	// Set trap flag on every thread
	void create_trap_break_point();
	
	// Remove trap flag from every thread
	void rever_trap_break_points();

	// Enter breakpoint
	void debug_break();
};

#endif