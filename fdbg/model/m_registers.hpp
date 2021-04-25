#pragma once
#ifndef FDBG_MODEL_M_REGISTERS_H_
#define FDBG_MODEL_M_REGISTERS_H_

#include <fdbg/win32_helpers/dbg_help.hpp>
#include <fdbg/model/model_interface.hpp>

struct registers_model : public view_model<registers_model>
{
	CONTEXT context;
	
	struct
	{
		struct
		{
			uint8_t display_mode = 0;
		} view;
	} config;
};

#endif