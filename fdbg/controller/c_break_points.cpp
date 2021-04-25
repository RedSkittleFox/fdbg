#include <fdbg/controller/c_break_points.hpp>

#include <fdbg/dbg/process.hpp>
#include <fdbg/dbg/threads.hpp>
#include <fdbg/dbg/debug_task_queue.hpp>

void break_points_controller::trigger()
{
	model().in_break = true;
	controller_manager::instance().break_point();
}

bool break_points_controller::triggered()
{
	return model().in_break;
}

void break_points_controller::continue_debug()
{
	if (model().in_break != false)
	{
		DWORD pi = process::instance().get_process();
		DWORD ti = threads::instance().current_thread().id;

		// ContinueDebugEvent has to be executed on the debugging thread.
		debug_task_queue::instance().push([pi, ti]
			{
				DWORD current_proc = process::instance().get_process();
				DWORD current_thread = threads::instance().current_thread().id;
				ContinueDebugEvent(pi, ti, DBG_CONTINUE);
			});
	}

	model().in_break = false;
}

void break_points_controller::create_break_point(void* address_)
{
	SIZE_T read_bytes;
	break_points_model::break_point bp;
	bp.address = address_;
	std::uint8_t ist; // Original instruction data

	// Get the data we are replacing
	ReadProcessMemory(process::instance().handle(),
		address_, &ist, 1, &read_bytes);
	bp.replaced_instruction = ist;

	// Write the INT to the pointer
	std::uint8_t INT = 0xCC; // Interrupt instruction
	WriteProcessMemory(process::instance().handle(), address_, &INT, 1, &read_bytes);
	FlushInstructionCache(process::instance().handle(), address_, 1);

	model().break_points.push_back(bp);
}

void break_points_controller::revert_break_point(void* address_)
{
	// Check if the breakpoint we've hit was put in place by us
	auto res = std::find_if(std::begin(model().break_points), std::end(model().break_points),
		[address_](const auto& e__)
		{
			return e__.address == address_;
		});

	// It wasn't our breakpoint. Return...
	if (res == std::end(model().break_points)) return;

	// Move back the instruction pointer
	{
		// Get context 
		CONTEXT context = {};
		context.ContextFlags = CONTEXT_ALL;
		bool v = GetThreadContext(threads::instance().current_thread().handle, &context);

		// Push instruction pointer backwards
		--context.Rip;
		v = SetThreadContext(threads::instance().current_thread().handle, &context);
	}

	// Remove step break points
	{
		SIZE_T read_bytes;
		WriteProcessMemory(process::instance().handle(), address_, &res->replaced_instruction,
			1, &read_bytes);


		FlushInstructionCache(process::instance().handle(), address_, 1);
	}

	model().break_points.erase(res);
}

void break_points_controller::create_trap_break_point()
{
	for (auto& t : threads::instance().get_threads())
	{
		// If debugging of the thread is disabled, skip it
		if (t.debug_enabled == false) continue;

		HANDLE hd = t.handle;

		// Set and Get thread context should be run on the debugging thread
		debug_task_queue::instance().push(
			[hd]()
			{
				CONTEXT context = {};
				context.ContextFlags = CONTEXT_ALL;

				GetThreadContext(hd, &context);
				// Set trap flag to one
				context.EFlags |= 0x0100;
				SetThreadContext(hd, &context);
			}
		);
	}
}

void break_points_controller::rever_trap_break_points()
{
	for (auto& t : threads::instance().get_threads())
	{
		debug_task_queue::instance().push([=]
			{
				// Reset context
				CONTEXT context = {};
				context.ContextFlags = CONTEXT_ALL;

				GetThreadContext(t.handle, &context);
				context.EFlags &= ~0x100; // Clear trap flag
				SetThreadContext(t.handle, &context);
			});
	}
}

void break_points_controller::debug_break()
{
	// Make sure we don't call it on the process that is already in a debug mode. 
	// Strange things will happen...
	if (!this->triggered() && process::instance().valid())
	{
		DebugBreakProcess(process::instance().handle());
	}
}
