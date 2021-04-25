#pragma once
#ifndef FDBG_CONTROLLER_C_BREAK_POINTS_H_
#define FDBG_CONTROLLER_C_BREAK_POINTS_H_

#include <fdbg/controller/controller_interface.hpp>
#include <fdbg/model/m_break_points.hpp>

struct break_points_controller : public controller<break_points_controller, break_points_model>
{
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