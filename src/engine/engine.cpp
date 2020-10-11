/**
 * Part of Epicinium
 * developed by A Bunch of Hacks.
 *
 * Copyright (c) 2017-2020 A Bunch of Hacks
 *
 * Epicinium is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Epicinium is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * [authors:]
 * Sander in 't Veld (sander@abunchofhacks.coop)
 * Daan Mulder (daan@abunchofhacks.coop)
 */
#include "engine.hpp"
#include "source.hpp"

#include "libs/SDL2/SDL.h"
#include "libs/imgui/imgui_sdl.h"

#include "settings.hpp"
#include "game.hpp"
#include "spritepattern.hpp"
#include "clock.hpp"
#include "palette.hpp"


EngineSDL::EngineSDL()
{
	// This prevents converting SIGINT into SDL_QUIT, because otherwise it is
	// impossible to quit the game with Ctrl+C when the game freezes.
	SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");

	// Default is OpenGL on Linux and Direct3D on Windows, but we always want
	// OpenGL.
	// TODO load a more recent version of OpenGL?
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");

	// Mac OS X users are used to Ctrl+click behaving like rightclick
	SDL_SetHint(SDL_HINT_MAC_CTRL_CLICK_EMULATE_RIGHT_CLICK, "1");

	// Initialize the timer and video SDL subsystems (video implicitly inits
	// events)
	SDL_ClearError();
	if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_AUDIO))
	{
		throw std::runtime_error("SDL_Init failed: "
			+ std::string(SDL_GetError()));
	}
}

EngineSDL::~EngineSDL()
{
	SDL_Quit();
}

// Preload gameplay music.
void Engine::runPreloadThread()
{
	Clip::get(Clip::Type::SPRING);
	Clip::get(Clip::Type::SUMMER);
	Clip::get(Clip::Type::AUTUMN);
	Clip::get(Clip::Type::WINTER);
	Clip::get(Clip::Type::NIGHTTIME);
	Clip::get(Clip::Type::ACTION);
}

Engine::Engine(Settings& settings) :
	_settings(settings),
	_displaysettings(_settings),
	_loop(*this, _settings.framerate.value()),
	_graphics(_settings),
	_renderer(_graphics),
	_camera(_settings, _graphics.width(), _graphics.height()),
	_mixer(_settings),
	_game(nullptr),
	_exitcode(ExitCode::DONE),
	_killer(*this),
	_display(_settings.display.value() > 0),
	_draw(true)
{
	_writer.install();

	_camera.install();

	Palette::installDefault();
	Palette::loadIndex();
	if (_settings.palette.defined())
	{
		Palette::installNamed(_settings.palette.value());
	}

	ImGuiSDL::Init(_graphics.getWindow());
	_graphics.install();
	_renderer.install();

	SpritePattern::preloadFromIndex();

	_mixer.install();

	_input.install();

	_library.load();
	_library.install();

	if (_settings.seed.defined())
	{
		srand(_settings.seed.value());
	}
	else
	{
		auto timestampMs = SteadyClock::milliseconds();
		srand(timestampMs);
	}

	// Preload gameplay music in a separate thread.
	// TODO does this need a try/catch black?
	_preloadThread = std::thread(&Engine::runPreloadThread);
}

Engine::~Engine()
{
	ImGuiSDL::Shutdown();
}

void Engine::doFirst()
{
	// If we want to do anything every second, do it here.
}

void Engine::doFrame()
{
	if (_preloadThread.joinable()) _preloadThread.detach();

	/* FLIP THE FRAME */
	if (_draw && _display)
	{
		_graphics.flip();
		_graphics.finish();
		_loop.flip();
	}

	/* RESET THE FRAME */
	_input.reset();

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		handle(event);
		_loop.event();
	}

	_input.moveMouse();

	ImGuiSDL::NewFrame(_graphics.getWindow());
	ImGui::NewFrame();
	if (_draw && _display)
	{
		_graphics.clear();
	}

	/* START OF UPDATES */
	startUpdates();

	_skineditor.updateIfEnabled();

	// Keep the game in scope until the end of this update step, in particular
	// until after _renderer.render().
	auto game = _game.lock();
	if (game)
	{
		_camera.update();
		game->update();
	}

	endUpdates();

	/*  END OF UPDATES  */
	_loop.expose();

	if (_draw && _display)
	{
		_graphics.prepare();
		_renderer.render();
		_graphics.update();
		ImGui::Render();
		ImGuiSDL::RenderDrawData();
	}
	else
	{
		_renderer.empty();
		ImGui::EndFrame();
	}

	_loop.finish();
	_graphics.finish();
	/* FRAME READY TO BE FLIPPED */

	_killer.check();
}

void Engine::startUpdates()
{
	if (_input.wasKeyPressed(SDL_SCANCODE_ESCAPE))
	{
		if (_input.isKeyHeld(SDL_SCANCODE_ALT)) _loop.stop();
		else if (auto game = _game.lock()) game->attemptQuit();
	}

	_mixer.update();
}

void Engine::endUpdates()
{
	// Nothing to do.
}

void Engine::handle(SDL_Event &event)
{
	ImGuiSDL::ProcessEvent(&event);
	ImGuiIO gui = ImGui::GetIO();

	switch (event.type)
	{
		case SDL_TEXTINPUT:
		{
			if (gui.WantCaptureKeyboard) break;
			_input.handle(event);
		}
		break;

		case SDL_TEXTEDITING:
		{
			if (gui.WantCaptureKeyboard) break;
			_input.handle(event);
		}
		break;

		case SDL_KEYDOWN:
		{
			if (gui.WantCaptureKeyboard) break;
			_input.handle(event);
		}
		break;

		case SDL_KEYUP:
		{
			// Always handle keyup to prevent stuck keys.
			// Superfluous keyups are ignored by Input.
			_input.handle(event);
		}
		break;

		case SDL_MOUSEBUTTONDOWN:
		{
			if (gui.WantCaptureMouse) break;
			_input.handle(event);
		}
		break;

		case SDL_MOUSEBUTTONUP:
		{
			// Always handle keyup to prevent stuck keys.
			// Superfluous keyups are ignored by Input.
			_input.handle(event);
		}
		break;

		case SDL_MOUSEWHEEL:
		{
			if (gui.WantCaptureMouse) break;
			_input.handle(event);
		}
		break;

		case SDL_QUIT:
		{
			if (_input.isKeyHeld(SDL_SCANCODE_ALT)) _loop.stop();
			else if (auto game = _game.lock()) game->attemptQuit();
			else _loop.stop();
		}
		break;

		case SDL_WINDOWEVENT:
		{
			switch (event.window.event)
			{
				case SDL_WINDOWEVENT_SHOWN:
				{
					_draw = true;
				}
				break;

				case SDL_WINDOWEVENT_HIDDEN:
				{
					_draw = false;
				}
				break;
			}
		}
		break;

		default: break;
	}
}

std::weak_ptr<Game> Engine::startGame(imploding_ptr<Game> gameptr)
{
	_game = std::move(gameptr);
	if (auto game = _game.lock()) game->load();
	return _game.remember();
}

void Engine::stopGame()
{
	_game = nullptr;
}

ExitCode Engine::run()
{
	_loop.run();

	return _exitcode;
}

void Engine::quit(ExitCode exitcode)
{
	_exitcode = exitcode;

	_mixer.stop();
	_loop.stop();
}
