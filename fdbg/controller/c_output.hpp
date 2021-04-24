#pragma once
#ifndef FDBG_CONTROLLER_C_OUTPUT_H_
#define FDBG_CONTROLLER_C_OUTPUT_H_

#include <string>

#include <fdbg/model/m_output.hpp>
#include <fdbg/controller/controller_interface.hpp>

class output_controller : public controller<output_controller, output_model>
{
public:
	output_controller();
	virtual void update() override final;

	void register_output_buffer(const std::string& name_);
	void print(const std::string& sink_, const std::string& output_);
	void printl(const std::string& sink_, const std::string& output_);
	void clear(const std::string& sink_);
	void clear();
};

#endif