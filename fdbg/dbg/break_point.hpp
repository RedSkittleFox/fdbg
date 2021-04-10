#pragma once
#ifndef FDBG_DBG_BREAK_POINT_H_
#define FDBG_DBG_BREAK_POINT_H_

class break_point
{
	bool m_break = false;

public:
	static break_point& instance();

public:
	bool triggered();
};

#endif