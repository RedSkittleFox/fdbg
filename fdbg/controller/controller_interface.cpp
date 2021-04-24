#include <fdbg/controller/controller_interface.hpp>

void controller_interface::update()
{
}

controller_manager& controller_manager::instance()
{
	static controller_manager inst;
	return inst;
}

void controller_manager::update()
{
	for (auto& con : m_controller_elements)
		con->update();
}

const std::vector<std::unique_ptr<controller_interface>>& controller_manager::controllers() const
{
	return m_controller_elements;
}
