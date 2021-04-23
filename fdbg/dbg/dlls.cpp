#include <fdbg/win32_helpers/windows.hpp>
#include <fdbg/dbg/output.hpp>
#include <fdbg/dbg/process.hpp>

#include <fdbg/dbg/dlls.hpp>

dlls& dlls::instance()
{
	static dlls dl;
	return dl;
}

void dlls::update()
{
}

void dlls::register_dll(const std::string& path_, void* start_address_, HANDLE imgage_, DWORD base_address_)
{
	std::string path;
	// Get path
	{
		size_t pos = path_.find_last_of('\\');
		path = std::string(path_, 0, pos);
	}

	DWORD64 base = SymLoadModule64(process::instance().handle(), nullptr, path_.c_str(), nullptr, base_address_, 0);
	auto err = GetLastError();
	bool loaded = false;
	if (base != false)
	{
		output::instance().print("Debug", " Loaded Module.");
		loaded = true;
	}

	m_dlls.push_back({ path_, start_address_, false, 0,  loaded});
}

std::string dlls::unregister_dll(void* start_address_)
{
	std::string fn;
	std::erase_if(m_dlls, [&](const auto& e__)
		{
			if (e__.start_address == start_address_)
			{
				fn = e__.filename;
				return true;
			}
			return false;
		});

	return fn;
}
