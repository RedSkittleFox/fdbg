#include <fstream>

#include <fdbg/controller/c_source_view.hpp>

static std::vector<std::string> source_file_loader(const std::string& file_)
{
	if (file_.empty()) return std::vector<std::string>();

	std::ifstream file;
	file.open(file_);
	if (file.bad()) return std::vector<std::string>();

	std::vector<std::string> data;
	std::string line;
	while (std::getline(file, line))
	{
		data.push_back(line);
	}

	return data;
}

source_view_controller::source_view_controller()
{
}

void source_view_controller::load_source_file(const std::string& name_)
{
	if (name_ == model().loaded_file.file_name) return;
	try_get_file();
	model().loaded_file.file_name = name_;
	model().loaded_file.file_data = std::async(std::launch::async, source_file_loader, name_);
}

void source_view_controller::update()
{
	// Check if we have to deal with any file loaded asynchronously
	try_get_file();
}

void source_view_controller::set_file(const std::string& name_, size_t line_number_, bool center_)
{
	auto& m = model();
	// Search for precached file
	auto r = m.files.find(name_);

	m.current_file.file_name = name_;
	m.current_file.line_number = line_number_;
	m.current_file.center = center_;
	if (r == std::end(m.files))
	{
		load_source_file(name_);
		m.current_file.file = nullptr;
	}
	else
	{
		m.current_file.file = &r->second;
	}
}

void source_view_controller::try_get_file()
{
	auto& s = model().loaded_file;
	if(s.file_data.valid())
	{
		auto& e = model().files[s.file_name] = s.file_data.get();
		auto& cf = model().current_file;
		if (cf.file_name == s.file_name)
		{
			cf.file = &e;
		}
	}

}
