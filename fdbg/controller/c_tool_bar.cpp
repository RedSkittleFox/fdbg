#include <fdbg/controller/c_tool_bar.hpp>

#include <fdbg/win32_helpers/windows.hpp>
#include <fdbg/dbg/break_points.hpp>
#include <fdbg/dbg/process.hpp>

void tool_bar_controller::update()
{
}

void tool_bar_controller::continue_execution()
{
	break_points::instance().continue_debug();
}

void tool_bar_controller::break_execution()
{
	break_points::instance().debug_break();
}

void tool_bar_controller::step_over()
{
	break_points::instance().create_trap_break_point();
	break_points::instance().continue_debug();
}
