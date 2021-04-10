#pragma once
#ifndef FDBG_DBG_MENU_BAR_H_
#define FDBG_DBG_MENU_BAR_H_

class menu_bar
{
public:
	static menu_bar& instance();
public:
	void update();
};

#endif