#include <fdbg/winmin.hpp>
#include <fdbg/dbg/process.hpp>
#include <fdbg/dbg/debug_task_queue.hpp>

#include <fdbg/dbg/break_points.hpp>
#include <fdbg/dbg/threads.hpp>

break_points& break_points::instance()
{
	static break_points bp;
	return bp;
}

bool break_points::triggered()
{
	return m_break;
}

void break_points::trigger()
{
	m_break = true;
}

void break_points::continue_debug()
{
	if (m_break != false)
	{
		DWORD pi = m_proc_id;
		DWORD ti = m_thread_id;
		debug_task_queue::instance().push([=] 
			{
				DWORD current_proc = process::instance().get_process();
				DWORD current_thread = threads::instance().current_thread().id;
				bool v = ContinueDebugEvent(pi, ti, DBG_CONTINUE); 
				int r = v;
			});
	}

	m_break = false;
}

void break_points::create_break_point(void* address_)
{
	SIZE_T read_bytes;
	break_point bp;
	bp.address = address_;

	std::uint8_t ist;
	
	bool v = ReadProcessMemory(process::instance().handle(),
		address_, &ist, 1, &read_bytes);

	DWORD r = GetLastError();

	bp.replaced_instruction = ist;
	std::uint8_t INT = 0xCC; // Interrupt instruction
	v = WriteProcessMemory(process::instance().handle(), address_, &INT, 1, &read_bytes);
	v = FlushInstructionCache(process::instance().handle(), address_, 1);
	m_break_points.push_back(bp);

}

void break_points::revert_break_point(void* address_)
{
	auto res = std::find_if(std::begin(m_break_points), std::end(m_break_points),
		[address_](const auto& e__)
		{
			return e__.address == address_;
		});

	// We didn't hit step break point
	if (res == std::end(m_break_points)) return;

	// Move back the instruction pointer

	// Get context 
	CONTEXT context = {};
	context.ContextFlags = CONTEXT_ALL;
	bool v = GetThreadContext(threads::instance().current_thread().handle, &context);

	// Push instruction pointer backwards
	--context.Rip;
	v = SetThreadContext(threads::instance().current_thread().handle, &context);

	// Remove step break points
	SIZE_T read_bytes;
	v = WriteProcessMemory(process::instance().handle(), address_, &res->replaced_instruction,
		1, &read_bytes);

	
	v = FlushInstructionCache(process::instance().handle(), address_, 1);
	m_break_points.erase(res);
}

void break_points::create_trap_break_point()
{
	
	for (auto& t : threads::instance().get_threads())
	{
		if (t.debug_enabled == false) continue;
		
		HANDLE hd = t.handle;
		
		debug_task_queue::instance().push(
			[hd]()
			{
				CONTEXT context = {};
				// Reset context
				context.ContextFlags = WOW64_CONTEXT_CONTROL;

				DWORD flags = 0;
				bool handle_test = GetHandleInformation(hd, &flags);
				bool v = GetThreadContext(hd, &context);
				DWORD err = GetLastError();
				context.EFlags |= 0x0100; // Trap flag

				v = SetThreadContext(hd, &context);
				int a = v;
			}
		);
	}
}

void break_points::rever_trap_break_points()
{
	for (auto& t : threads::instance().get_threads())
	{
		debug_task_queue::instance().push([=]
			{
				// Reset context
				CONTEXT context{};
				context.ContextFlags = CONTEXT_ALL;

				GetThreadContext(t.handle, &context);
				context.EFlags &= ~0x100; // Clear trap flag
				SetThreadContext(t.handle, &context);
			});
	}
}

void break_points::set_debug_identifiers(DWORD proc_, DWORD thread_)
{
	m_proc_id = proc_;
	m_thread_id = thread_;
}

void break_points::debug_break()
{
	if (!this->triggered() && process::instance().valid())
	{
		DebugBreakProcess(process::instance().handle());
	}
}

