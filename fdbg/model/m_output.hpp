#pragma once
#ifndef FDBG_MODEL_M_OUTPUT_H_
#define FDBG_MODEL_M_OUTPUT_H_

#include <fdbg/model/model_interface.hpp>

struct output_model : public view_model<output_model>
{
	std::vector<std::pair<std::string, std::string>> output_buffers;
};

#endif