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
#pragma once
#include "header.hpp"

#include <thread>

#include "engineloop.hpp"
#include "enginekiller.hpp"
#include "displaysettings.hpp"
#include "camera.hpp"
#include "graphics.hpp"
#include "collector.hpp"
#include "mixer.hpp"
#include "input.hpp"
#include "writer.hpp"
#include "library.hpp"
#include "skineditor.hpp"
#include "implodingptr.hpp"
#include "exitcode.hpp"

union SDL_Event;
class Settings;
class Game;
class Screenshot;


class EngineSDL
{
public:
	EngineSDL();
	~EngineSDL();
};

class Engine : protected EngineLoop::Owner
{
private:
	static void runPreloadThread();

public:
	Engine(Settings& settings);
	Engine(const Engine&) = delete;
	Engine(Engine&&) = delete;
	Engine& operator=(const Engine&) = delete;
	Engine& operator=(Engine&&) = delete;
	virtual ~Engine();

protected:
	EngineSDL _sdl;
	Settings& _settings;
	DisplaySettings _displaysettings;
	EngineLoop _loop;
	Graphics _graphics;
	Collector _renderer;
	Camera _camera;
	Mixer _mixer;
	Input _input;
	Writer _writer;
	Library _library;
	SkinEditor _skineditor;
	imploding_ptr<Game> _game;
	std::shared_ptr<Screenshot> _nextScreenshot;
	std::shared_ptr<Screenshot> _screenshot;
	ExitCode _exitcode;
	EngineKiller _killer;

	bool _display;
	bool _draw;

	std::thread _preloadThread;

	virtual void doFirst() override;
	virtual void doFrame() override;

	virtual void startUpdates();
	virtual void endUpdates();

	void handle(SDL_Event &event);

	virtual std::weak_ptr<Game> startGame(imploding_ptr<Game> game);
	virtual void stopGame();

public:
	ExitCode run();

	void quit(ExitCode exitcode = ExitCode::DONE);
};
