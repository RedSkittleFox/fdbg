#pragma once
#ifndef FDBG_MODEL_M_PROCESS_LAUNCHER_H_
#define FDBG_MODEL_M_PROCESS_LAUNCHER_H_

#include <future>
#include <string>

#include <fdbg/model/model_interface.hpp>

struct process_launcher_model : public view_model<process_launcher_model>
{
	std::future<std::string> executable_path;
	std::future<std::string> env_path;
	bool popped = false;
	struct
	{
		std::string path;
		std::string params;
		std::string env;
	} config;
};

#endif