#ifndef FDBG_CONTROLLER_C_STACK_H_
#define FDBG_CONTROLLER_C_STACK_H_
#pragma once

#include <fdbg/controller/controller_interface.hpp>
#include <fdbg/model/m_stack.hpp>

struct stack_controller : public controller<stack_controller, stack_model>
{
	void break_point();
	stack_model::call_stack_entry current_stack_entry();
};

#endif