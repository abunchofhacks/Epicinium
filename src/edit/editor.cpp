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
#include "editor.hpp"
#include "source.hpp"

#include "libs/imgui/imgui_sdl.h"

#include "clock.hpp"
#include "coredump.hpp"
#include "version.hpp"
#include "loginstaller.hpp"
#include "map.hpp"
#include "locator.hpp"
#include "palette.hpp"
#include "recording.hpp"
#include "spritepattern.hpp"
#include "language.hpp"
#include "editortheme.hpp"


int main(int argc, char* argv[])
{
	CoreDump::enable();

	std::string logname = "editor";
	std::string mapname = "";

	Settings settings("settings-editor.json", argc, argv);

	for (int i = 1; i < argc; i++)
	{
		const char* arg = argv[i];
		if (strncmp(arg, "-", 1) == 0)
		{
			// Setting argument, will be handled by Settings.
		}
		else
		{
			mapname = arg;
		}
	}

	if (settings.logname.defined())
	{
		logname = settings.logname.value();
	}
	else settings.logname.override(logname);

	std::cout << "[ Epicinium Editor ]";
	std::cout << " (" << logname << " v" << Version::current() << ")";
	std::cout << std::endl << std::endl;

	if (settings.dataRoot.defined())
	{
		LogInstaller::setRoot(settings.dataRoot.value());
		Recording::setRoot(settings.dataRoot.value());
	}

	LogInstaller(settings).install();

	if (settings.dataRoot.defined())
	{
		Map::setAuthoredRoot(settings.dataRoot.value());
		Locator::setAuthoredRoot(settings.dataRoot.value());
		Palette::setAuthoredRoot(settings.dataRoot.value());
	}

	LOGI << "Start v" << Version::current();

	// Enable internationalization.
	Language::use(settings);

	{
		Editor editor(settings, mapname);
		editor.run();
	}

	std::cout << std::endl << std::endl << "[ Done ]" << std::endl;
	return 0;
}

EditorSDL::EditorSDL()
{
	SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
	SDL_ClearError();
	if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_AUDIO))
	{
		throw std::runtime_error("SDL_Init failed: "
			+ std::string(SDL_GetError()));
	}
}

EditorSDL::~EditorSDL()
{
	SDL_Quit();
}

Editor::Editor(Settings& settings, const std::string& mapname) :
	_settings(settings),
	_displaysettings(_settings),
	_loop(*this, _settings.framerate.value()),
	_graphics(_settings),
	_renderer(_graphics),
	_camera(_graphics.width(), _graphics.height(), _settings.getEditorScale()),
	_mixer(_settings)
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

	EditorTheme::apply();

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

	_mapeditor.reset(new MapEditor(*this, *this, mapname));
}

Editor::~Editor()
{
	ImGuiSDL::Shutdown();
}

void Editor::run()
{
	_loop.run();
}

void Editor::doFirst()
{
	EditorTheme::apply();

	_mapeditor->beforeFirstUpdateOfEachSecond();
}

void Editor::doFrame()
{
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
	_graphics.clear();
	/* START OF UPDATES */

	_camera.update();
	_mixer.update();

	_mapeditor->update();

	_skineditor.updatePalettes();
	_skineditor.updateSkins();

	/*  END OF UPDATES  */
	_loop.expose();

	_graphics.prepare();
	_renderer.render();
	_graphics.update();
	ImGui::Render();
	ImGuiSDL::RenderDrawData();
	_graphics.flip();
}

void Editor::handle(SDL_Event &event)
{
	ImGuiSDL::ProcessEvent(&event);
	ImGuiIO gui = ImGui::GetIO();

	switch (event.type)
	{
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
			quit();
		}
		break;

		default: break;
	}
}

void Editor::quit()
{
	_mapeditor->quit();
}

void Editor::onConfirmQuit()
{
	_loop.stop();
}

std::weak_ptr<Game> Editor::startGame(imploding_ptr<Game> /**/)
{
	// Not implemented.
	return std::weak_ptr<Game>();
}

std::weak_ptr<Game> Editor::startChallenge(const Challenge&, const std::string&)
{
	// Not implemented.
	return std::weak_ptr<Game>();
}

std::weak_ptr<Game> Editor::startGame(
		const Player&, const std::string&,
		uint32_t /**/)
{
	// Not implemented.
	return std::weak_ptr<Game>();
}

std::weak_ptr<Game> Editor::startTutorial(
		const Player&, const std::string&,
		uint32_t /**/)
{
	// Not implemented.
	return std::weak_ptr<Game>();
}

std::weak_ptr<Game> Editor::startReplay(
		const Role&, const std::string&,
		uint32_t /**/)
{
	// Not implemented.
	return std::weak_ptr<Game>();
}

std::weak_ptr<Game> Editor::startDiorama()
{
	// Not implemented.
	return std::weak_ptr<Game>();
}

std::weak_ptr<HostedGame> Editor::startHostedGame(
		const std::vector<Player>& /**/,
		const std::vector<VisionType>& /**/,
		const std::vector<std::string>& /**/,
		const std::vector<Bot>& /**/,
		bool /**/,
		const std::string& /**/, const std::string& /**/)
{
	// Not implemented.
	return std::weak_ptr<HostedGame>();
}

void Editor::stopGame()
{
	// Not implemented.
}

void Editor::openPaletteEditor()
{
	return _skineditor.openPaletteEditor("");
}
