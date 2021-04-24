#include <fdbg/model/model_interface.hpp>

uint8_t model_interface::s_id_tracker = 0;

///////////////////////////////////////////////////////////////////////////////
// model_manager implementations
//

model_manager& model_manager::instance()
{
	static model_manager inst;
	return inst;
}

const std::vector<std::unique_ptr<model_interface>>& model_manager::models() const
{
	return m_models;
}
