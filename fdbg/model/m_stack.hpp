#pragma once
#ifndef FDBG_MODEL_M_STACK_H_
#define FDBG_MODEL_M_STACK_H_

#include <vector>
#include <cstdint>

#include <fdbg/win32_helpers/dbg_help.hpp>
#include <fdbg/model/model_interface.hpp>

struct stack_model : public model<stack_model>
{
    struct call_stack_entry
    {
        std::string name;
        void* address;
        bool external;
        size_t line_number;
        std::string source_file;
        std::vector<imagehlp_symbol_variable*> variables;
    };

    std::vector<call_stack_entry> call_stack;
    size_t current_stack_entry;
};

struct stack_trace_model : public view_model<stack_trace_model> {};
struct stack_variables_model : public view_model<stack_variables_model> {};

#endif