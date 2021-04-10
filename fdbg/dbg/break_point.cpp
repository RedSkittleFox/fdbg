#include <fdbg/dbg/break_point.hpp>

break_point& break_point::instance()
{
	static break_point bp;
	return bp;
}

bool break_point::triggered()
{
	return m_break;
}
