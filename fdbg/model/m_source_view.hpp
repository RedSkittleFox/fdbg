#pragma once
#ifndef FDBG_MODEL_M_SOURCE_VIEW_H_
#define FDBG_MODEL_M_SOURCE_VIEW_H_

#include <unordered_map>
#include <vector>
#include <future>
#include <string>
#include <numeric>

#include <fdbg/model/model_interface.hpp>

struct source_view_model : public view_model<source_view_model>
{
	std::unordered_map<std::string, std::vector<std::string>> files;
	
	// Curently viewed file
	struct
	{
		std::string file_name;
		typename typename decltype(files)::value_type::second_type* file;

		static constexpr size_t default_line_number = std::numeric_limits<size_t>::max();
		size_t line_number;
		bool center;
	} current_file;

	// Data related to async file loading
	struct
	{
		std::string file_name;
		std::future<std::vector<std::string>> file_data;
	} loaded_file;
};

#endif