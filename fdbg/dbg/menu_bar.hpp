#pragma once
#ifndef FDBG_DBG_MENU_BAR_H_
#define FDBG_DBG_MENU_BAR_H_

class menu_bar
{
public:
	static menu_bar& instance();

public:
	struct
	{
		struct 
		{
			bool break_on_first_instruction = true;
		} debug;
	} config;

public:
	void update();
};

#endif