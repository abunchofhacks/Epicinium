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
#include "libs/jsoncpp/json.h"

#include "engineloop.hpp"
#include "board.hpp"
#include "level.hpp"
#include "settings.hpp"
#include "displaysettings.hpp"
#include "camera.hpp"
#include "camerafocus.hpp"
#include "graphics.hpp"
#include "collector.hpp"
#include "mixer.hpp"
#include "input.hpp"
#include "writer.hpp"
#include "descriptor.hpp"
#include "tiletoken.hpp"
#include "unittoken.hpp"
#include "bible.hpp"
#include "library.hpp"
#include "skinner.hpp"
#include "skineditor.hpp"
#include "color.hpp"

class Cursor;
class ChangeSet;


enum class PoolType : uint8_t;
enum class Popup : uint8_t;

class EditorSDL
{
public:
	EditorSDL();
	~EditorSDL();
};

class Editor final : virtual private EngineLoop::Owner
{
private:
	struct HazardPaint
	{
		int8_t gas = 0;
		int8_t radiation = 0;

		bool frostbite = false;
		bool firestorm = false;
		bool bonedrought = false;
		bool death = false;
	};

public:
	Editor(Settings& settings, const std::string& filename = "");
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
	std::shared_ptr<CameraFocus> _camerafocus; // (unique ownership)
	Mixer _mixer;
	Input _input;
	Writer _writer;
	Library _library;

	std::string _filename;
	bool _unsavedCached;

	PoolType _pooltype;
	PoolType _savedpooltype;
	Bible _bible;
	Skinner _skinner;

	int _cols;
	int _rows;
	Board _board;
	Level _level;
	Board _saved;

	int _playercount;

	Popup _activepopup;
	Popup _openedpopup;

	SkinEditor _skineditor;

	Descriptor _paintdesc;
	TileToken _tilepaint;
	UnitToken _unitpaint;
	HazardPaint _hazardpaint;

	std::unique_ptr<Cursor> _cursor;

	virtual void doFirst() override;
	virtual void doFrame() override;

	void handle(SDL_Event &event);

	void updatePaintMode();
	void updatePaintBrush();
	void updateMenuBar();
	void updateParameters();
	void updateGlobals();
	void updatePopup();

	void expandTop();
	void expandLeft();
	void expandRight();
	void expandBottom();
	void cropTop();
	void cropLeft();
	void cropRight();
	void cropBottom();

	void store(const Position& topleft, ChangeSet& changes);
	void restore(const ChangeSet& changes);

	void generateBiomes();

	void clear();
	void onNewDimensions(int cols, int rows);
	void loadEmpty();

	void load();
	void loadFrom();
	void loadCopyFrom();
	void onOpenFileFilename(const std::string &fname);
	void onCopyFileFilename(const std::string &fname);
	void loadFromFile(const std::string& fname);
	void loaded();

	void save();
	void saveAs();
	void saveCopyAs();
	void onSaveAsFilename(const std::string &fname);
	void onSaveCopyAsFilename(const std::string &fname);
	void saveToFile(const std::string& fname);
	void saved();
	bool unsaved();

	void changeRuleset();
	void onChangeRuleset(const PoolType& pooltype, const std::string& name);

	void quit();
	void onConfirmQuit();

public:
	void run();
};
