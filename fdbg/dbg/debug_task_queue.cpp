#include <fdbg/dbg/debug_task_queue.hpp>

debug_task_queue& debug_task_queue::instance()
{
	static debug_task_queue i;
	return i;
}

void debug_task_queue::execute()
{
	while (!m_tasks.empty())
	{
		auto task = std::move(m_tasks.front());
		m_tasks.pop();
		std::invoke(task);
	}
}
