#include <fdbg/controller/c_registers.hpp>

void registers_controller::break_point()
{
	CONTEXT c = {};
	c.ContextFlags = CONTEXT_ALL;
	GetThreadContext(dbg_thread(), &c);
	model().context = c;
}
