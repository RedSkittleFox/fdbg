#include <fdbg/controller/c_threads.hpp>

void threads_controller::register_thread(DWORD id_, HANDLE handle_, std::string name_)
{
	model().threads.push_back(
		threads_model::thread_entry
		{
			.id = id_, 
			.handle = handle_, 
			.name = name_, 
			.debug_enabled = true
		}
	);
}

void threads_controller::unregister_thread(DWORD id_)
{
	std::erase_if(model().threads, [=](const auto& e__)
		{
			return e__.id == id_;
		});
}

threads_model::thread_entry& threads_controller::current_thread()
{
	// TODO: Do a check
	return model().threads[model().current_thread];
}

void threads_controller::set_current_thread(DWORD id_)
{
	auto res = std::find_if(std::begin(model().threads), std::end(model().threads), 
		[=](const auto& e__)
		{
			return e__.id == id_;
		});

	model().current_thread = std::distance(std::begin(model().threads), res);
}

decltype(threads_model::threads)& threads_controller::get_threads()
{
	return model().threads;
}

void threads_controller::update_handles()
{
}
