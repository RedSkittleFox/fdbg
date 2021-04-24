#pragma once
#ifndef FDBG_CONTROLLER_C_PROCESS_SELECTOR_H_
#define FDBG_CONTROLLER_C_PROCESS_SELECTOR_H_

#include <fdbg/controller/controller_interface.hpp>
#include <fdbg/model/m_process_selector.hpp>

struct process_selector_controller : public controller<process_selector_controller, process_selector_model>
{
public:
	void update();
	void update_process_list();
};

#endif