#include <sstream>
#include <type_traits>
#include <fdbg/imgui/imgui.h>
#include <fdbg/imgui/imgui_stdlib.h>

#include <fdbg/dbg/threads.hpp>
#include <fdbg/dbg/registers.hpp>
#include <fdbg/controller/c_break_points.hpp>
#include <fdbg/dbg/process.hpp>

registers& registers::instance()
{
	static registers r;
	return r;
}

void registers::update()
{
	ImGuiWindowFlags flags = 0;

	if (!mvc<break_points_controller>().triggered())
	{
		flags |= ImGuiWindowFlags_NoInputs;
		// ImGui::SetNextWindowBgAlpha(0.5);
	}

	if (ImGui::Begin("Registers", &m_visible, ImGuiWindowFlags_MenuBar))
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("View"))
			{
				static const char* preview = "";
				if (config.view.display_mode == 0)
				{
					preview = "Binary";
				}
				else if (config.view.display_mode == 1)
				{
					preview = "Hexadecimal";
				}
				else if (config.view.display_mode == 2)
				{
					preview = "Signed Decimal";
				}
				else if (config.view.display_mode == 3)
				{
					preview = "Unsigned Decimal";
				}

				if (ImGui::BeginCombo("##format", preview))
				{
					if (ImGui::Selectable("Binary", config.view.display_mode == 0))
					{
						config.view.display_mode = 0;
					}
					if (ImGui::Selectable("Hexadecimal", config.view.display_mode == 1))
					{
						config.view.display_mode = 1;
					}
					if (ImGui::Selectable("Signed Decimal", config.view.display_mode == 2))
					{
						config.view.display_mode = 2;
					}
					if (ImGui::Selectable("Unsigned Decimal", config.view.display_mode == 3))
					{
						config.view.display_mode = 3;
					}
					ImGui::EndCombo();
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		auto to_string = [=]<class T>(T val__) -> std::string
		{
			if ((config.view.display_mode == 1 || config.view.display_mode == 0 ) && sizeof(T) != 1)
			{
				std::stringstream ss;
				ss << std::hex << std::showbase << val__;
				return ss.str();
			}
			else if (config.view.display_mode == 2)
			{
				return std::to_string(std::bit_cast<std::make_signed_t<T>>(val__));
			}
			else if (config.view.display_mode == 3 || sizeof(T) == 1)
			{
				return std::to_string(std::bit_cast<std::make_unsigned_t<T>>(val__));
			}
		};

		if (ImGui::BeginTable("RegisterTable", 2))
		{
			ImGui::TableSetupColumn("Register");
			ImGui::TableSetupColumn("Value");

			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableHeadersRow();

			// TODO:
			// Fix high 8 byte
			// Fix exception on invalid input
			auto create_row = [&](std::string register_, uint64_t& value_, bool& modified, bool use_half_ = false, bool use_octa_ = false)
			{
				std::string value = to_string(value_);

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				bool open;
				open = ImGui::TreeNodeEx(register_.c_str(), ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_Bullet);
				ImGui::TableSetColumnIndex(1);
				if (ImGui::InputText("##qword", &value, ImGuiInputTextFlags_CharsHexadecimal))
				{
					if (!value.empty())
					{
						value_ = std::stoll(value);
						modified = true;
					}
				}

				if (open)
				{
					uint32_t dword = static_cast<uint32_t>(value_);
					uint16_t word = static_cast<uint16_t>(value_);
					
					if (use_half_)
					{
						// DWORD
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						register_[0] = 'e';
						ImGui::TreeNodeEx(register_.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth);
						ImGui::TableSetColumnIndex(1);
						value = to_string(dword);
						if (ImGui::InputText("##dword", &value, ImGuiInputTextFlags_CharsHexadecimal))
						{
							if (!value.empty())
							{
								dword = std::stoll(value);
								value_ &= 0xFFFFFFFF00000000l;
								value_ |= static_cast<uint64_t>(dword);
								modified = true;
							}
						}

						// WORD
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						register_[0] = register_[1];
						register_[1] = register_[2];
						register_[2] = 0;
						ImGui::TreeNodeEx(register_.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth);
						ImGui::TableSetColumnIndex(1);
						value = to_string(word);
						if (ImGui::InputText("##word", &value, ImGuiInputTextFlags_CharsHexadecimal))
						{
							if (!value.empty())
							{
								word = std::stoll(value);
								value_ &= 0xFFFFFFFFFFFF0000l;
								value_ |= static_cast<uint64_t>(word);
								modified = true;
							}
						}

						// Half
						if (use_octa_)
						{
							uint8_t b = static_cast<uint8_t>(value_);

							// FIXME: 
							uint8_t h = static_cast<uint8_t>(value_ >> 1);
							
							// High
							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							register_[1] = 'h';
							ImGui::TreeNodeEx(register_.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth);
							ImGui::TableSetColumnIndex(1);
							value = to_string(h);
							if (ImGui::InputText("##sh", &value, ImGuiInputTextFlags_CharsHexadecimal))
							{
								if (!value.empty())
								{
									h = std::stoll(value);
									value_ &= 0xFFFFFFFFFFFF00FFl;
									value_ |= (static_cast<uint64_t>(h) << 1);
									modified = true;
								}
							}

							// Bottom
							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							register_[1] = 'b';
							ImGui::TreeNodeEx(register_.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth);
							ImGui::TableSetColumnIndex(1);
							value = to_string(b);
							if (ImGui::InputText("##sb", &value, ImGuiInputTextFlags_CharsHexadecimal))
							{
								if (!value.empty())
								{
									b = std::stoll(value);
									value_ &= 0xFFFFFFFFFFFFFF00l;
									value_ |= static_cast<uint64_t>(b);
									modified = true;
								}
							}
						}
					}
					
					ImGui::TreePop();
				}
			};

			bool modified = false;
			create_row("rax", m_context.Rax, modified, true, true);
			create_row("rbx", m_context.Rbx, modified, true, true);
			create_row("rcx", m_context.Rcx, modified, true, true);
			create_row("rdx", m_context.Rdx, modified, true, true);
			create_row("rsi", m_context.Rsi, modified, true);
			create_row("rdi", m_context.Rdi, modified, true);
			create_row("rbp", m_context.Rbp, modified, true);
			create_row("rsp", m_context.Rsp, modified, true);
			create_row("r8", m_context.R8, modified);
			create_row("r9", m_context.R9, modified);
			create_row("r10", m_context.R10, modified);
			create_row("r11", m_context.R11, modified);
			create_row("r12", m_context.R12, modified);
			create_row("r13", m_context.R13, modified);
			create_row("r14", m_context.R14, modified);
			create_row("r15", m_context.R15, modified);

			// Update context
			if (modified)
			{
				SetThreadContext(threads::instance().current_thread().handle, &m_context);
			}

			ImGui::EndTable();
		}
	}
	ImGui::End();
}

void registers::update_context()
{
	m_context.ContextFlags = CONTEXT_ALL;
	GetThreadContext(threads::instance().current_thread().handle, &m_context);
}
