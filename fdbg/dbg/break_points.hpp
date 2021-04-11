#pragma once
#ifndef FDBG_DBG_BREAK_POINTS_H_
#define FDBG_DBG_BREAK_POINTS_H_

#include <vector>
#include <string>

class break_points
{
public:
	// Inserted by user
	struct break_point
	{
		bool enabled;
		std::string source;
		size_t line;

		std::string dll;
		size_t offset;
		std::byte replaced_instruction;
	};
	
	// Inserted by the debugger
	struct step_break_point
	{
		std::string dll;
		size_t offset;
		std::byte replaced_instruction;
	};

private:
	bool m_break = false;
	std::vector<break_point> m_break_points;
	std::vector<step_break_point> m_step_break_points;

public:
	static break_points& instance();

public:
	bool triggered();
};

#endif