#pragma once
#ifndef FDBG_DBG_DLLS_H_
#define FDBG_DBG_DLLS_H_

#include <string>
#include <filesystem>
#include <vector>
#include <fdbg/winmin.hpp>

class dlls
{
public:
	typedef struct
	{
		// We don't need properites of std::filesystem::path here
		std::string filename;
		void* start_address;
		bool debug_enabled;
		DWORD64 module_handle;
		IMAGEHLP_MODULE64 module_info;
	} dll_entry;

private:
	std::vector<dll_entry> m_dlls;
	size_t m_current_dll;

public:
	static dlls& instance();

public:
	void update();
	void register_dll(const std::string& path_, void* start_address_, DWORD base_address_);
	std::string unregister_dll(void* start_address_);
};

#endif