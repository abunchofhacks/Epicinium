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

#include "board.hpp"
#include "level.hpp"
#include "camerafocus.hpp"
#include "descriptor.hpp"
#include "tiletoken.hpp"
#include "unittoken.hpp"
#include "skinner.hpp"
#include "bible.hpp"
#include "color.hpp"

class Cursor;
class ChangeSet;
class GameOwner;
class Settings;
class Screenshot;


enum class PoolType : uint8_t;
enum class Popup : uint8_t;

class ChallengeEditData;

class MapEditor
{
public:
	class Owner
	{
	protected:
		Owner() = default;

	public:
		Owner(const Owner& /**/) = delete;
		Owner(Owner&& /**/) = delete;
		Owner& operator=(const Owner& /**/) = delete;
		Owner& operator=(Owner&& /**/) = delete;
		virtual ~Owner() = default;

		virtual Settings& settings() = 0;

		virtual bool isTakingScreenshot() = 0;

		virtual bool hasWorkshop() { return false; };
		virtual void openWorkshop(const std::string& /*mapname*/,
			const std::string& /*rulesetname*/) {};
		virtual void closeWorkshop() {};

		virtual void openPaletteEditor() = 0;

		virtual void onConfirmQuit() = 0;
	};

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
	MapEditor(Owner& owner, GameOwner& gameowner,
		const std::string& mapname = "");
	~MapEditor();

	MapEditor(const MapEditor&) = delete;
	MapEditor(MapEditor&&) = delete;
	MapEditor& operator=(const MapEditor&) = delete;
	MapEditor& operator=(MapEditor&&) = delete;

private:
	Owner& _owner;
	GameOwner& _gameowner;

	std::string _mapname;
	bool _unsavedCached;

	bool _showRulesetEditor;
	std::string _rulesetname;
	std::vector<char> _rulesetbuffer;
	bool _rulesetvalid;
	bool _rulesetunsaved;
	bool _rulesetsaving;
	std::string _ruleseterrormessage;

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
	std::unique_ptr<ChallengeEditData> _challenge;

	Popup _activepopup;
	Popup _openedpopup;
	std::vector<std::string> _cachedMapNames;
	std::vector<std::string> _cachedRulesetNames;

	Descriptor _paintdesc;
	TileToken _tilepaint;
	UnitToken _unitpaint;
	HazardPaint _hazardpaint;

	std::shared_ptr<CameraFocus> _camerafocus; // (unique ownership)
	std::unique_ptr<Cursor> _cursor;

public:
	void beforeFirstUpdateOfEachSecond();
	void update();

protected:
	void updatePaintMode();
	void updatePaintBrush();
	void updateMenuBar();
	void updateParameters();
	void updateGlobals();
	void updatePopup();

	void updateRulesetEditor();

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
	void onOpenMapName(const std::string& mapname);
	void onCopyMapName(const std::string& mapname);
	void loadFromFile(const std::string& mapname);
	void loaded();
	void onLoadFailure();

	void save();
	void saveAs();
	void saveCopyAs();
	void onSaveAsMapName(const std::string& mapname);
	void onSaveCopyAsMapName(const std::string& mapname);
	void saveToFile(const std::string& mapname);
	void saved();
	bool unsaved();

	void changeRuleset();
	void onChangeRuleset(const PoolType& pooltype,
		const std::string& rulesetname);

	void playTestGame();
	void playVersusAI();

	void switchToRulesetEditor();
	void switchToPaletteEditor();

public:
	void quit();
	void onConfirmQuit();

	std::shared_ptr<Screenshot> prepareScreenshotOfMap();
};
