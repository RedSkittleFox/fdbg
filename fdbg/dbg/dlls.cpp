#include <fdbg/winmin.hpp>
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

void dlls::register_dll(const std::string& path_, void* start_address_, DWORD base_address_)
{
	// Try to load debug information
	DWORD64 base = SymLoadModule64(process::instance().handle(), nullptr, path_.c_str(), nullptr, base_address_, 0);

	IMAGEHLP_MODULE64 module_info;
	module_info.SizeOfStruct = sizeof(module_info);
	bool res = SymGetModuleInfo64(process::instance().handle(), base, &module_info);

	if (res == true && module_info.SymType == SymPdb)
	{
		output::instance().print("Debug", " Symbols loaded.");
	}

	m_dlls.push_back({ path_, start_address_, false, base, module_info });
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
