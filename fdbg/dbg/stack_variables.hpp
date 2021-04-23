#pragma once
#ifndef FDBG_DBG_STACK_VARIABLES_H_
#define FDBG_DBG_STACK_VARIABLES_H_

class stack_variables
{
    bool m_enabled = true;

public:
    static stack_variables& instance();

    void update();
};

#endif