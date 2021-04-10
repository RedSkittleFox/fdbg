#ifndef FDBG_DBG_OUTPUT_H_
#define FDBG_DBG_OUTPUT_H_
#pragma once

#include <vector>
#include <string>

class output
{
	bool m_visible = true;

	std::vector<std::pair<std::string, std::string>> m_output_buffers;
public:
	static output& instance();

public:
	output();
	void update();
	void register_output_buffer(const std::string& name_);
	void print(const std::string& sink_, const std::string& output_);
	void printl(const std::string& sink_, const std::string& output_);
	void clear(const std::string& sink_);
	void clear();
};

#endif
