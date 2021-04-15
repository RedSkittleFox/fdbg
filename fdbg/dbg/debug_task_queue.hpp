// Author:		Marcin Poloczek (RedSkittleFox)
// Contact:		RedSkittleFox@gmail.com
// Copyright:	Refer to project's license.
// Purpose:		Right now only the thread that started the process can use debugger loop
//				this is why this queue is needed. It's gonna execute delayed functions 
//				on the debug thread such as create process.
#pragma once
#ifndef FDBG_DBG_DEBUG_TASK_QUEUE_H_
#define FDBG_DBG_DEBUG_TASK_QUEUE_H_

#include <queue>
#include <functional>

class debug_task_queue
{
	std::queue<std::function<void()>> m_tasks;

public:
	static debug_task_queue& instance();

public:
	// Push callable to the queue
	template<class T> void push(T callable_)
	{
		m_tasks.push(callable_);
	};

	// Execute functions and clear the queue. 
	// This should only be called once a frame by 
	// debugging thread.
	void execute();
};

#endif