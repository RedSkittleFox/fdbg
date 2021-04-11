#include <fdbg/dbg/dlls.hpp>

dlls& dlls::instance()
{
	static dlls dl;
	return dl;
}

void dlls::update()
{
}

void dlls::register_dll(const std::string& path_, void* start_address_)
{
	m_dlls.push_back({ path_, start_address_, false });
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
