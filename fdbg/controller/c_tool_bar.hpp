#pragma once
#ifndef FDBG_CONTROLLER_C_TOOL_BAR_H_
#define FDBG_CONTROLLER_C_TOOL_BAR_H_

#include <fdbg/controller/controller_interface.hpp>
#include <fdbg/model/m_tool_bar.hpp>

struct tool_bar_controller : public controller<tool_bar_controller, tool_bar_model>
{
	void update();

	// Tools
	void continue_execution();
	void break_execution();
	void step_over();
};

#endif