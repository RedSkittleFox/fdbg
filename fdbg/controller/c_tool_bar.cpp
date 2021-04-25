#include <fdbg/controller/c_tool_bar.hpp>

#include <fdbg/win32_helpers/windows.hpp>
#include <fdbg/controller/c_break_points.hpp>
#include <fdbg/dbg/process.hpp>

void tool_bar_controller::update()
{
}

void tool_bar_controller::continue_execution()
{
	mvc<break_points_controller>().continue_debug();
}

void tool_bar_controller::break_execution()
{
	mvc<break_points_controller>().debug_break();
}

void tool_bar_controller::step_over()
{
	mvc<break_points_controller>().create_trap_break_point();
	mvc<break_points_controller>().continue_debug();
}
