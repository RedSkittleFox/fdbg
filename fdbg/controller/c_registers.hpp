#pragma once
#ifndef FDBG_CONTROLLER_C_REGISTERS_H_
#define FDBG_CONTROLLER_C_REGISTERS_H_

#include <fdbg/controller/controller_interface.hpp>
#include <fdbg/model/m_registers.hpp>

struct registers_controller : public controller<registers_controller, registers_model>
{
	void break_point();
};

#endif