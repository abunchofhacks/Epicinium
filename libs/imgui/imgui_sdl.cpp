#include "imgui.h"

#include "imgui_sdl.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl2.h"


bool ImGuiSDL::Init(SDL_Window* window)
{
	return ImGui_ImplSDL2_InitForOpenGL(window, nullptr)
		&& ImGui_ImplOpenGL2_Init();
}

void ImGuiSDL::Shutdown()
{
	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
}

void ImGuiSDL::NewFrame(SDL_Window* window)
{
	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
}

void ImGuiSDL::RenderDrawData()
{
	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}

bool ImGuiSDL::ProcessEvent(SDL_Event* event)
{
	return ImGui_ImplSDL2_ProcessEvent(event);
}
