#include <fdbg/dbg/process.hpp>

process& process::instance()
{
	static process p;
	return p;
}

void process::attach(DWORD p_)
{
	detach();
	WIN_ASSERT(DebugActiveProcess(p_), "Failed to attach to the process.");
	WIN_ASSERT(DebugSetProcessKillOnExit(false));

	m_process = p_;
}

void process::detach()
{
	if (m_process != 0)
	{
		WIN_ASSERT(DebugActiveProcessStop(m_process), "Unable to stop debugging the process.");
		m_process = 0;
	}
}

DWORD process::get_process() const noexcept
{
	return m_process;
}

bool process::valid()
{
	return m_process != 0;
}
