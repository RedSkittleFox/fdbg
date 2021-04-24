#include <algorithm>
#include <numeric>

#include <fdbg/controller/c_process_selector.hpp>

void process_selector_controller::update()
{
	if (model().enum_process_result.valid())
	{
		model().processes = model().enum_process_result.get();

		// Update filers
		model().filtering.filter_entries = std::vector<size_t>(model().processes.size());
		std::iota(std::begin(model().filtering.filter_entries), std::end(model().filtering.filter_entries), 0);
	}
}

void process_selector_controller::update_process_list()
{
	if (!model().enum_process_result.valid())
	{
		model().enum_process_result = std::async(std::launch::async, enum_proceseses);
	}
}
