#include <fdbg/dbg/break_points.hpp>

break_points& break_points::instance()
{
	static break_points bp;
	return bp;
}

bool break_points::triggered()
{
	return m_break;
}
