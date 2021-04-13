#pragma once
#ifndef FDBG_DBG_REGISTERS_H_
#define FDBG_DBG_REGISTERS_H_


#include <fdbg/winmin.hpp>

class registers
{
	bool m_visible;

	CONTEXT m_context;
public:
	static registers& instance();

public:
	struct
	{
		struct
		{
			// binary, hex, decimal
			uint8_t display_mode = 2;
		} view;
	} config;

public:
	void update();
	void update_context();
};

#endif