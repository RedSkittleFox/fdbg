#include <fdbg/win32_helpers/dbg_help.hpp>
#include <fdbg/controller/c_break_points.hpp>

#include <fdbg/dbg/process.hpp>
#include <fdbg/dbg/debug_task_queue.hpp>
#include <fdbg/controller/c_threads.hpp>

void break_points_controller::trigger()
{
	model().in_break = true;
	controller_manager::instance().break_point();
}

bool break_points_controller::triggered()
{
	return model().in_break;
}

// TODO: Fixme
void break_points_controller::continue_debug()
{
	if (model().in_break != false)
	{
		DWORD pi = process::instance().get_process();
		DWORD ti = mvc<threads_controller>().current_thread().id;

		// ContinueDebugEvent has to be executed on the debugging thread.
		debug_task_queue::instance().push([pi, ti]
			{
				DWORD current_proc = process::instance().get_process();
				DWORD current_thread = mvc<threads_controller>().current_thread().id;
				ContinueDebugEvent(pi, ti, DBG_CONTINUE);
			});
	}

	model().in_break = false;
}

void break_points_controller::create_break_point(void* address_)
{
	SIZE_T read_bytes;
	
	std::uint8_t ist; // Original instruction data

	// Get the data we are replacing
	ReadProcessMemory(process::instance().handle(),
		address_, &ist, 1, &read_bytes);

	// Write the INT to the pointer
	std::uint8_t INT = 0xCC; // Interrupt instruction
	WriteProcessMemory(process::instance().handle(), address_, &INT, 1, &read_bytes);
	FlushInstructionCache(process::instance().handle(), address_, 1);

	break_points_model::break_point bp
	{
		.source = "",
		.line = 0u,
		.address = address_,
		.replaced_instruction = ist,
		.single_hit = true,
		.try_revert = false
	};

	model().break_points.push_back(bp);
}

void break_points_controller::revert_break_point(void* address_)
{
	// Restore try revert breakpoints
	{
		auto& bp = model().break_points;
		for (auto& b : bp)
		{
			if (b.try_revert == true)
			{
				SIZE_T read_bytes;
				std::uint8_t INT = 0xCC; // Interrupt instruction
				WriteProcessMemory(process::instance().handle(), b.address, &INT,
					1, &read_bytes);

				FlushInstructionCache(process::instance().handle(), b.address, 1);
				b.try_revert = false;
			}
		}
	}

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
		bool v = GetThreadContext(dbg_thread(), &context);

		// Push instruction pointer backwards
		--context.Rip;
		v = SetThreadContext(dbg_thread(), &context);
	}

	// Remove Breakpoint
	{
		SIZE_T read_bytes;
		WriteProcessMemory(process::instance().handle(), address_, &res->replaced_instruction,
			1, &read_bytes);

		FlushInstructionCache(process::instance().handle(), address_, 1);
	}
	
	// Handle breakpoint meta
	{
		// if we are once-hit breakpoint, remove itself
		if(res->single_hit)
		{
			model().break_points.erase(res);
		}
		else
		{
			res->try_revert = true;
		}
	}
}

void break_points_controller::create_break_point(const std::string& filename_, size_t line_)
{
	DWORD disp;

	auto ptr = imagehlp_get_line_address(filename_, line_);

	if (ptr == nullptr) return;

	SIZE_T read_bytes;

	std::uint8_t ist; // Original instruction data

	// Get the data we are replacing
	ReadProcessMemory(process::instance().handle(),
		ptr, &ist, 1, &read_bytes);

	// Write the INT to the pointer
	std::uint8_t INT = 0xCC; // Interrupt instruction
	WriteProcessMemory(process::instance().handle(), ptr, &INT, 1, &read_bytes);
	FlushInstructionCache(process::instance().handle(), ptr, 1);

	break_points_model::break_point bp
	{
		.source = filename_,
		.line = line_,
		.address = ptr,
		.replaced_instruction = ist,
		.single_hit = false,
		.try_revert = false
	};

	model().break_points.push_back(bp);
}

void break_points_controller::revert_break_point(const std::string& filename_, size_t line_)
{
	for (auto& bp : model().break_points)
	{
		if (line_ == bp.line && bp.source == filename_)
		{
			revert_break_point(bp.address);
			return;
		}
	}
}

void break_points_controller::create_trap_break_point()
{
	for (auto& t : mvc<threads_controller>().get_threads())
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
	for (auto& t : mvc<threads_controller>().get_threads())
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
