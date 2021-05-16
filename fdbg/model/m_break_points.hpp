#pragma once
#ifndef FDBG_MODEL_M_BREAK_POINTS_H_
#define FDBG_MODEL_M_BREAK_POINTS_H_

#include <fdbg/model/model_interface.hpp>

struct break_points_model : public view_model<break_points_model>
{
	struct break_point
	{
		std::string source;					// Source file
		size_t line;						// Source line
		void* address;						// Address (global)
		std::uint8_t replaced_instruction;  // Replaced instruction data
		bool single_hit;
		bool try_revert;
	};

	bool in_break;
	std::vector<break_point> break_points;

	struct
	{
		bool treat_as_breakpoint;
	} trap_breakpoint;
};

#endif