#include <fdbg/controller/c_output.hpp>

output_controller::output_controller()
{
    auto& m = model();
    m.output_buffers = std::vector<std::pair<std::string, std::string>>();

	register_output_buffer("Debug");
	register_output_buffer("General");
}

void output_controller::update()
{
}

void output_controller::register_output_buffer(const std::string& name_)
{
	model().output_buffers.push_back({ name_, std::string("") });
}

void output_controller::print(const std::string& sink_, const std::string& output_)
{
    auto res = std::find_if(std::begin(model().output_buffers), std::end(model().output_buffers), [&](const auto& e__) -> bool { return e__.first == sink_; });
    if (res == std::end(model().output_buffers))
    {
        this->printl("General", std::string("Unknown sink: ") + sink_);
        return;
    }
    res->second.append(output_);
}

void output_controller::printl(const std::string& sink_, const std::string& output_)
{
    auto res = std::find_if(std::begin(model().output_buffers), std::end(model().output_buffers), [&](const auto& e__) -> bool { return e__.first == sink_; });
    if (res == std::end(model().output_buffers))
    {
        this->printl("General", std::string("Unknown sink: ") + sink_);
        return;
    }
    res->second.push_back('\n');
    res->second.append(output_);
}

void output_controller::clear(const std::string& sink_)
{
    auto res = std::find_if(std::begin(model().output_buffers), std::end(model().output_buffers), [&](const auto& e__) -> bool { return e__.first == sink_; });
    if (res == std::end(model().output_buffers))
    {
        this->printl("General", std::string("Unknown sink: ") + sink_);
        return;
    }
    res->second.clear();
}

void output_controller::clear()
{
    for (auto& s : model().output_buffers)
        s.second.clear();
}
