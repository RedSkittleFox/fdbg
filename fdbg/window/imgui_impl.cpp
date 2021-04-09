#include <fdbg/window/imgui.hpp>

#include <fdbg/winmin.hpp>

#include <fdbg/imgui/imgui.h>
#include <fdbg/imgui/imgui_impl_win32.h>
#include <fdbg/imgui/imgui_impl_opengl2.h>

#include <gl/GL.h>
#include <gl/GLU.h>

void create_imgui(HWND wnd)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); 
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	(void)io;
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(wnd);
	ImGui_ImplWin32_EnableDpiAwareness();

	ImGui_ImplOpenGL2_Init();
}

void update_imgui()
{
	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void render(HWND wnd)
{
	ImGui::Render();
	RECT rect;
	GetWindowRect(wnd, &rect);
	glViewport(0, 0, rect.right - rect.left, rect.bottom - rect.top);

	glClearColor(0.9, 0.9, 0.9, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}

void destroy_imgui()
{
	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}