#pragma once
#ifndef FDBG_CONTROLLER_C_SOURCE_VIEW_H_
#define FDBG_CONTROLLER_C_SOURCE_VIEW_H_

#include <string>

#include <fdbg/model/m_source_view.hpp>
#include <fdbg/controller/controller_interface.hpp>

class source_view_controller : public controller<source_view_controller, source_view_model>
{
public:
	source_view_controller();
	void load_source_file(const std::string& name_);
	virtual void update() override final;
	void set_file(const std::string& name_, 
		// This is awful...
		size_t line_number_ = mvc<source_view_model>().current_file.default_line_number,
		bool center_ = false);
private:
	void try_get_file();
};

#endif