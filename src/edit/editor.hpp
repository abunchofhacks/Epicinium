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

#include "libs/SDL2/SDL.h"

#include "engineloop.hpp"
#include "gameowner.hpp"
#include "settings.hpp"
#include "displaysettings.hpp"
#include "camera.hpp"
#include "camerafocus.hpp"
#include "graphics.hpp"
#include "collector.hpp"
#include "mixer.hpp"
#include "input.hpp"
#include "writer.hpp"
#include "library.hpp"
#include "mapeditor.hpp"
#include "skineditor.hpp"


class EditorSDL
{
public:
	EditorSDL();
	~EditorSDL();
};

class Editor final : virtual private EngineLoop::Owner,
	private MapEditor::Owner, private GameOwner
{
public:
	Editor(Settings& settings, const std::string& mapname = "");
	~Editor();

	Editor(const Editor&) = delete;
	Editor(Editor&&) = delete;
	Editor& operator=(const Editor&) = delete;
	Editor& operator=(Editor&&) = delete;

private:
	EditorSDL _sdl;
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

	std::unique_ptr<MapEditor> _mapeditor;

	SkinEditor _skineditor;

	virtual void doFirst() override;
	virtual void doFrame() override;

	virtual Settings& settings() override { return _settings; }

	virtual void openPaletteEditor() override;

	virtual bool isTakingScreenshot() override { return false; }

	virtual void onConfirmQuit() override;

	virtual std::weak_ptr<Game> startGame(imploding_ptr<Game> game) override;
	virtual std::weak_ptr<Game> startChallenge(
		const Challenge& challenge, const std::string& name) override;
	virtual std::weak_ptr<Game> startGame(
		const Player& player, const std::string& rulesetname,
		uint32_t planningTime) override;
	virtual std::weak_ptr<Game> startTutorial(
		const Player& player, const std::string& rulesetname,
		uint32_t planningTime) override;
	virtual std::weak_ptr<Game> startReplay(
		const Role& role, const std::string& rulesetname,
		uint32_t planningTime) override;
	virtual std::weak_ptr<Game> startDiorama() override;

	virtual std::weak_ptr<HostedGame> startHostedGame(
		const std::vector<Player>& colors,
		const std::vector<VisionType>& visiontypes,
		const std::vector<std::string>& usernames,
		const std::vector<Bot>& bots,
		bool hasObservers,
		const std::string& mapname, const std::string& rulesetname) override;

	virtual void stopGame() override;

	void handle(SDL_Event &event);

	void quit();

public:
	void run();
};
