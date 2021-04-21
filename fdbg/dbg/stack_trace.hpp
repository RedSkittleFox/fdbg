#pragma once
#ifndef FDBG_DBG_STACK_TRACE_H_
#define FDBG_DBG_STACK_TRACE_H_

#include <string>
#include <vector>

class stack_trace
{
    bool m_enabled = true;
public:
    static stack_trace& instance();

public:
    struct call_stack_entry
    {
        std::string name;
        void* address;
        bool external;
        size_t line_number;
        std::string source_file;
        // Tells if this is the stack entry we are currently debugging (is currently selected)
    };

private:
    std::vector<call_stack_entry> m_call_stack;
    size_t m_current_stack_entry = 0;

public:
    void update();
    void update_data();
    call_stack_entry current_stack_entry();
};

#endif