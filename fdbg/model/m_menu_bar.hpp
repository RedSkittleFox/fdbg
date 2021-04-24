///////////////////////////////////////////////////////////////////////////////
// Author:		Marcin Poloczek (RedSkittleFox)
// Contact:		RedSkittleFox@gmail.com
// Copyright:	Refer to project's license.
// 

#ifndef FDBG_MODEL_M_MENU_BAR_H_
#define FDBG_MODEL_M_MENU_BAR_H_
#pragma once

#include <fdbg/model/model_interface.hpp>

///////////////////////////////////////////////////////////////////////////////
// Purpose:	Menu Bar Model
// 
struct menu_bar_model : public view_model<menu_bar_model>
{
	struct
	{
		// Indicates weather to place break point 
		// on the first instruction in main.
		bool break_on_first_instruction = true;
	} debug;
};

#endif