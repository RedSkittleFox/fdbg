#pragma once
#ifndef FDBG_DBG_DEBUG_TASK_QUEUE_H_
#define FDBG_DBG_DEBUG_TASK_QUEUE_H_

#include <queue>
#include <functional>

// Right now only the thread that started the process can use debugger loop
// this is why this queue is needed. It's gonna execute delayed functions 
// on the debug thread such as create process.
class debug_task_queue
{
	std::queue<std::function<void()>> m_tasks;

public:
	static debug_task_queue& instance();

public:
	template<class T>
	void push(T callable_)
	{
		m_tasks.push(callable_);
	};

	void execute();
};

#endif