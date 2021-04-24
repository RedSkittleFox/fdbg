#include <fdbg/model/m_menu_bar.hpp>
#include <fdbg/win32_helpers/windows.hpp>
#include <fdbg/dbg/process.hpp>
#include <fdbg/dbg/debug_task_queue.hpp>
#include <fdbg/dbg/break_points.hpp>


process& process::instance()
{
	static process p;
	return p;
}

void process::start(const std::string& path_, std::string cmd_, const std::string& env_)
{
	this->detach();
	m_should_kill_process = true;
	auto startup = [this, path_, cmd_, env_]
	{
		auto cmd = cmd_;
		STARTUPINFO si{ 0 };
		PROCESS_INFORMATION pi{ 0 };
		si.cb = sizeof(si);
		CreateProcess(path_.c_str(), cmd == "" ? nullptr : cmd.data(), nullptr, nullptr, false,
			DEBUG_ONLY_THIS_PROCESS, // TODO: Should this be DEBUG_ONLY_THIS_PROCESS?
			nullptr, env_ == "" ? nullptr : env_.c_str(), &si, &pi);

		this->m_process = pi.dwProcessId;
		this->m_handle = pi.hProcess;
		WIN_ASSERT(DebugSetProcessKillOnExit(false));
		

	};
	debug_task_queue::instance().push(startup);
}

void process::attach(DWORD p_)
{
	// Disable breaking on first instruction when we are attaching!
	// menu_bar::instance().config.debug.break_on_first_instruction = false;
	model_manager::instance().model<menu_bar_model>().debug.break_on_first_instruction = false;

	detach();
	m_should_kill_process = false;
	WIN_ASSERT(DebugActiveProcess(p_), "Failed to attach to the process.");
	WIN_ASSERT(DebugSetProcessKillOnExit(false));

	m_process = p_;
	m_handle = OpenProcess(PROCESS_ALL_ACCESS, false, m_process);

	// TODO: Output dbg message
	// Attach symbol
}

void process::detach()
{
	if (m_process != 0)
	{
		break_points::instance().continue_debug();
		DWORD pid = m_process;
		if(!should_kill())
			debug_task_queue::instance().push([pid] { DebugActiveProcessStop(pid); });
		else
			debug_task_queue::instance().push([pid] 
				{
					HANDLE proc = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
					TerminateProcess(proc, 0);
					CloseHandle(proc);
					DebugActiveProcessStop(pid);
				});
		
		HANDLE proc = OpenProcess(PROCESS_ALL_ACCESS, false, m_process);
		DebugBreakProcess(proc);
		CloseHandle(proc);
		m_process = 0;
	}
}

DWORD process::get_process() const noexcept
{
	return m_process;
}

HANDLE process::handle()
{
	return m_handle;
}

void process::handle(HANDLE hndl_)
{
	m_handle = hndl_;
}

bool process::valid()
{
	return m_process != 0;
}

bool process::should_kill()
{
	return m_should_kill_process;
}
