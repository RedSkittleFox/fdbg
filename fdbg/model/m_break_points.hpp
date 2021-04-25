#pragma once
#ifndef FDBG_MODEL_M_BREAK_POINTS_H_
#define FDBG_MODEL_M_BREAK_POINTS_H_

#include <fdbg/model/model_interface.hpp>

struct break_points_model : public view_model<break_points_model>
{
	struct break_point
	{
		bool enabled;			// Breakpoint is enabled
		std::string source;		// Source file
		size_t line;			// Source line

		// TODO:
		// bool single_hit;

		std::string dll;		// Dll file
		void* address;			// Address (global)

		// Replaced instruction data
		std::uint8_t replaced_instruction;
	};

	bool in_break;
	std::vector<break_point> break_points;
};

#endif