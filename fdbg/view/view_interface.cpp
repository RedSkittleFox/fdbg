#include <fdbg/view/view_interface.hpp>

///////////////////////////////////////////////////////////////////////////////
// view_manager implementations
//

view_manager& view_manager::instance()
{
    static view_manager inst;
    return inst;
}

void view_manager::draw()
{
    for (auto& e : m_view_elements)
    {
        if(e->vmodel().visible)
            e->draw();
    }
}

const std::vector<std::unique_ptr<view_interface>>& view_manager::views() const
{
    return m_view_elements;
}
