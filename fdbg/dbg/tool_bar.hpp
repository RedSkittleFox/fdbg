#pragma once
#ifndef FDBG_DBG_TOOL_BAR_H_
#define FDBG_DBG_TOOL_BAR_H_

class tool_bar
{

public:
	static tool_bar& instance();
	void update();
};

#endif