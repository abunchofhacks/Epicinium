#pragma once

struct SDL_Window;
typedef union SDL_Event SDL_Event;


namespace ImGuiSDL
{
	bool Init(SDL_Window* window);

	void Shutdown();

	void NewFrame(SDL_Window* window);

	void RenderDrawData();

	bool ProcessEvent(SDL_Event* event);
}
