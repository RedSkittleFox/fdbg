#pragma once
#ifndef FDBG_DBG_PROCESS_LAUNCHER_H_
#define FDBG_DBG_PROCESS_LAUNCHER_H_

#include <array>
#include <fdbg/winmin.hpp>

class process_launcher
{
	bool m_visible;
private:
	struct defaults_t
	{
		std::array<char, MAX_PATH> path;
		std::array<char, MAX_PATH> params;
		std::array<char, MAX_PATH> env;
	};

	defaults_t defaults;
public:
	static process_launcher& instance();

public:
	void update();
	void pop();
};

#endif