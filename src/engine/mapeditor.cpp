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
#include "mapeditor.hpp"
#include "source.hpp"

#include "libs/imgui/imgui_sdl.h"

#include "level.hpp"
#include "surface.hpp"
#include "cursor.hpp"
#include "changeset.hpp"
#include "automaton.hpp"
#include "map.hpp"
#include "locator.hpp"
#include "typenamer.hpp"
#include "parseerror.hpp"
#include "treetype.hpp"
#include "system.hpp"
#include "library.hpp"
#include "input.hpp"
#include "settings.hpp"
#include "writer.hpp"
#include "validation.hpp"
#include "gameowner.hpp"
#include "localgame.hpp"
#include "bot.hpp"
#include "ai.hpp"
#include "aichallenge.hpp"
#include "difficulty.hpp"
#include "camera.hpp"
#include "screenshot.hpp"


static std::vector<Player> allPlayers;

enum class Popup : uint8_t
{
	NONE = 0,
	QUIT,
	NEW,
	OPEN_FILE,
	COPY_FILE,
	SAVE_AS,
	SAVE_COPY_AS,
	CHANGE_RULESET,
	SAVE_RULESET,
	ERROR,
};

class ChallengeEditData
{
public:
	bool advanced = false;
	std::array<char, 20> name = {};
	int stars = 1;
	std::array<char, 50> greeting = {};
	std::array<char, 500> description = {};
	std::array<char, 500> objective = {};
	std::array<char, 50> one = {};
	std::array<char, 50> two = {};
	std::array<char, 50> three = {};
	std::array<char, 50> sendoff = {};

	void update();

	void load(const Json::Value& root);
	void store(Json::Value& root);
};

MapEditor::MapEditor(Owner& owner, GameOwner& gameowner,
		const std::string& mapname) :
	_owner(owner),
	_gameowner(gameowner),
	_mapname(mapname),
	_showRulesetEditor(false),
	_rulesetbuffer(32000, '\0'),
	_rulesetvalid(false),
	_rulesetunsaved(false),
	_rulesetsaving(false),
	_ruleseterrormessage("Loading..."),
	_pooltype(PoolType::MULTIPLAYER),
	_savedpooltype(PoolType::MULTIPLAYER),
	_bible(),
	_skinner(_bible),
	_cols(20),
	_rows(13),
	_board(_bible),
	_level(_bible, _skinner),
	_saved(_bible),
	_playercount(2),
	_activepopup(Popup::NONE),
	_openedpopup(Popup::NONE),
	_cursor(nullptr)
{
	_board.clear(_cols, _rows);
	_level.clear(_cols, _rows);

	allPlayers.push_back(Player::NONE);
	for (Player player : getPlayers(PLAYER_MAX))
	{
		allPlayers.push_back(player);
	}

	_camerafocus.reset(new CameraFocus(_owner.settings(), Surface::WIDTH));
	_camerafocus->set(_level.centerPoint());
	_camerafocus->load(_level.topleftPoint(), _level.bottomrightPoint());
	_camerafocus->stopActing();

	_cursor.reset(new Cursor(_board, _bible));
	_cursor->setState(Cursor::State::ACTIVE);

	// Load the most recent bible. This works because the Level and the Boards
	// have a reference to _bible, which remains valid when we change its data.
	_bible = Library::getBible(Library::nameCurrentBible());

	if (!_mapname.empty())
	{
		load();
	}
	else
	{
		loadEmpty();
	}
}

MapEditor::~MapEditor() = default;

void MapEditor::beforeFirstUpdateOfEachSecond()
{
	_unsavedCached = unsaved();
}

std::shared_ptr<Screenshot> MapEditor::prepareScreenshotOfMap()
{
	_camerafocus->set(_level.centerPoint());

	// Make the screenshot slightly smaller than the map so we can cut off
	// the ugly corners a little bit.
	int w = Camera::get()->scale() * (_cols - 1) * Surface::WIDTH;
	int h = Camera::get()->scale() * (_rows - 1) * Surface::HEIGHT;
	return std::make_shared<Screenshot>(w, h, "map");
}

inline bool isValidPoolTypeForWorkshop(const PoolType& pooltype)
{
	switch (pooltype)
	{
		case PoolType::NONE: return false;
		case PoolType::MULTIPLAYER: return true;
		case PoolType::CUSTOM: return true;
		case PoolType::DIORAMA: return false;
	}
	return false;
}

void MapEditor::update()
{
	if (_activepopup == Popup::NONE
		&& Input::get()->wasKeyPressed(SDL_SCANCODE_ESCAPE))
	{
		quit();
	}

	_camerafocus->update();

	if (_cursor && _cursor->state() == Cursor::State::ACTIVE)
	{
		Input* input = Input::get();
		Point point(input->mousePoint());
		Position position(Surface::convert(point));
		Cell index = _level.index(position);
		_cursor->set(index);
	}

	if (_cursor->state() == Cursor::State::ACTIVE
		&& _mapname != "colorsample"
		&& !(_showRulesetEditor && _mapname.empty()))
	{
		updatePaintMode();
		updatePaintBrush();
		updateParameters();
		updateGlobals();
	}
	else if (_cursor->state() == Cursor::State::WAITING
		&& !Input::get()->isKeyHeld(SDL_SCANCODE_LMB)
		&& !Input::get()->isKeyHeld(SDL_SCANCODE_RMB))
	{
		_cursor->setState(Cursor::State::ACTIVE);
	}

	if (_showRulesetEditor)
	{
		updateRulesetEditor();
	}

	updateMenuBar();
	updatePopup();

	if (_challenge)
	{
		_challenge->update();
	}

	if (_cursor && !_owner.isTakingScreenshot())
	{
		_cursor->update();
	}

	_level.update();
}

void MapEditor::updateMenuBar()
{
	if (Input::get()->isKeyHeld(SDL_SCANCODE_CTRL))
	{
		if (Input::get()->wasKeyPressed(SDL_SCANCODE_S))
		{
			if (Input::get()->isKeyHeld(SDL_SCANCODE_ALT))
			{
				saveCopyAs();
			}
			else if (_mapname.empty()
				|| Input::get()->isKeyHeld(SDL_SCANCODE_SHIFT))
			{
				saveAs();
			}
			else
			{
				save();
			}
		}
		else if (Input::get()->wasKeyPressed(SDL_SCANCODE_O))
		{
			if (Input::get()->isKeyHeld(SDL_SCANCODE_ALT))
			{
				loadCopyFrom();
			}
			else
			{
				loadFrom();
			}
		}
		else if (Input::get()->wasKeyPressed(SDL_SCANCODE_N))
		{
			clear();
		}
		else if (Input::get()->wasKeyPressed(SDL_SCANCODE_U))
		{
			if (_owner.hasWorkshop()
				&& ((_mapname == "colorsample")
					|| (!_mapname.empty() && _mapname != "colorsample"
						&& isValidPoolTypeForWorkshop(_pooltype))
					|| (_mapname.empty() && !_rulesetname.empty())))
			{
				if (!_mapname.empty())
				{
					save();
				}
				_owner.openWorkshop(_mapname, _rulesetname);
			}
		}
		else if (Input::get()->wasKeyPressed(SDL_SCANCODE_T))
		{
			playTestGame();
		}
		else if (Input::get()->wasKeyPressed(SDL_SCANCODE_P))
		{
			playVersusAI();
		}
		else if (Input::get()->wasKeyPressed(SDL_SCANCODE_J))
		{
			_pooltype = PoolType::NONE;
			_savedpooltype = PoolType::NONE;
			_board.clear(_cols, _rows);
			_level.clear(_cols, _rows);
			loadEmpty();
			_mapname = "";
			switchToRulesetEditor();
		}
		else if (Input::get()->wasKeyPressed(SDL_SCANCODE_H))
		{
			switchToPaletteEditor();
		}
	}

	if (ImGui::BeginMainMenuBar())
	{
		const char *fileheader = _unsavedCached ? "Map*" : "Map ";
		if (ImGui::BeginMenu(fileheader))
		{
			std::stringstream dispname;
			if (_mapname.empty())
				dispname << "(new map)";
			else
				dispname << "(map: " << _mapname << ")";
			ImGui::MenuItem(dispname.str().c_str(), NULL, false, false);
			ImGui::Separator();
			if (ImGui::MenuItem("New...", "Ctrl+N"))
			{
				clear();
			}
			if (ImGui::MenuItem("Open...", "Ctrl+O"))
			{
				loadFrom();
			}
			if (ImGui::MenuItem("Copy...", "Ctrl+Alt+O"))
			{
				loadCopyFrom();
			}
			if (ImGui::MenuItem("Save", "Ctrl+S"))
			{
				if (_mapname.empty())
				{
					saveAs();
				}
				else
				{
					save();
				}
			}
			if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
			{
				saveAs();
			}
			if (ImGui::MenuItem("Save Copy As...", "Ctrl+Alt+S"))
			{
				saveCopyAs();
			}
			if (_owner.hasWorkshop() && !_mapname.empty()
				&& _mapname != "colorsample")
			{
				if (ImGui::MenuItem("Publish to Steam Workshop...", "Ctrl+U"))
				{
					if (isValidPoolTypeForWorkshop(_pooltype))
					{
						save();
						_owner.openWorkshop(_mapname, _rulesetname);
					}
					else
					{
						changeRuleset();
					}
				}
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Quit...", "Esc"))
			{
				quit();
			}
			ImGui::EndMenu();
		}

		if (!_mapname.empty() && _mapname != "colorsample"
			&& ImGui::BeginMenu("Play"))
		{
			if (ImGui::MenuItem("Play Test Game", "Ctrl+T"))
			{
				playTestGame();
			}
			if (ImGui::MenuItem("Play Versus AI", "Ctrl+P"))
			{
				playVersusAI();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Ruleset"))
		{
			if (ImGui::MenuItem("Switch to Ruleset Editor", "Ctrl+J"))
			{
				_pooltype = PoolType::NONE;
				_savedpooltype = PoolType::NONE;
				_board.clear(_cols, _rows);
				_level.clear(_cols, _rows);
				loadEmpty();
				_mapname = "";
				switchToRulesetEditor();
			}
			if (_owner.hasWorkshop() && _mapname.empty()
				&& !_rulesetname.empty())
			{
				if (ImGui::MenuItem("Publish to Steam Workshop...", "Ctrl+U"))
				{
					_owner.openWorkshop(_mapname, _rulesetname);
				}
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Palette"))
		{
			if (ImGui::MenuItem("Switch to Palette Editor", "Ctrl+H"))
			{
				switchToPaletteEditor();
			}
			if (_owner.hasWorkshop() && _mapname == "colorsample")
			{
				if (ImGui::MenuItem("Publish to Steam Workshop...", "Ctrl+U"))
				{
					_owner.openWorkshop(_mapname, _rulesetname);
				}
			}
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void MapEditor::updatePaintMode()
{
	{
		_paintdesc.position = Surface::convert(Input::get()->mousePoint());
	}

	if (ImGui::Begin("Paint Brush", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		int md = static_cast<int>(_paintdesc.type);
		ImGui::RadioButton("Tile",   &md, static_cast<int>(Descriptor::Type::TILE));
		ImGui::RadioButton("Ground", &md, static_cast<int>(Descriptor::Type::GROUND));
		ImGui::RadioButton("Air"  ,  &md, static_cast<int>(Descriptor::Type::AIR));
#if EDITOR_DEPRECATED_ENABLED
		ImGui::RadioButton("Hazards",&md, static_cast<int>(Descriptor::Type::CELL));
#endif
		_paintdesc.type = static_cast<Descriptor::Type>(md);

		ImGui::Separator();

		std::stringstream txt;
		if (_paintdesc.position.col >= 0 && _paintdesc.position.col < _cols
			&& _paintdesc.position.row >= 0 && _paintdesc.position.row < _rows)
		{
			txt << _paintdesc.position;
		}
		else
		{
			txt << "(out of bounds)";
		}
		ImGui::Text("%s", txt.str().c_str());

		Square* square = _level.cell(_paintdesc.position);
		ImGui::Text("Humidity: %i", square->humidity());
	}
	ImGui::End();
}

void MapEditor::updatePaintBrush()
{
	Cell index = _board.cell(_paintdesc.position);
	if (Input::get()->isKeyHeld(SDL_SCANCODE_LMB) && !index.edge())
	{
		switch (_paintdesc.type)
		{
			case Descriptor::Type::TILE:
			{
				const TileToken& existing = _board.tile(index);

				if (existing != _tilepaint && _tilepaint.type != TileType::NONE)
				{
					Change change(Change::Type::TRANSFORMED, _paintdesc, _tilepaint);
					_board.enact(change);
					_level.enact(change, nullptr);
					_level.cell(_paintdesc.position)->cleanup();
					generateBiomes();
					_level.setBorderLight(1, 0);
				}
			}
			break;

			case Descriptor::Type::GROUND:
			case Descriptor::Type::AIR:
			{
				const UnitToken& existing = _board.unit(index, _paintdesc.type);

				if (existing && existing != _unitpaint)
				{
					Change change(Change::Type::EXITED, _paintdesc);
					_board.enact(change);
					_level.enact(change, nullptr);
					_level.cell(_paintdesc.position)->cleanup();
				}

				if (_unitpaint.type != UnitType::NONE
					&& (!existing || existing != _unitpaint))
				{
					Change change(Change::Type::ENTERED, _paintdesc, _unitpaint);
					_board.enact(change);
					_level.enact(change, nullptr);
				}
			}
			break;

			case Descriptor::Type::CELL:
			{
				if (_hazardpaint.gas != _board.gas(index))
				{
					Change change(Change::Type::GAS, _paintdesc);
					int8_t diff = _hazardpaint.gas - _board.gas(index);
					change.xGas(diff);
					_board.enact(change);
					_level.enact(change, nullptr);
				}
				if (_hazardpaint.radiation != _board.radiation(index))
				{
					Change change(Change::Type::RADIATION, _paintdesc);
					int8_t diff = _hazardpaint.radiation - _board.gas(index);
					change.xRadiation(diff);
					_board.enact(change);
					_level.enact(change, nullptr);
				}
				if (_hazardpaint.frostbite != _board.frostbite(index))
				{
					Change change(Change::Type::FROSTBITE, _paintdesc);
					change.xFrostbite(_hazardpaint.frostbite);
					_board.enact(change);
					_level.enact(change, nullptr);
				}
				if (_hazardpaint.firestorm != _board.firestorm(index))
				{
					Change change(Change::Type::FIRESTORM, _paintdesc);
					change.xFirestorm(_hazardpaint.firestorm);
					_board.enact(change);
					_level.enact(change, nullptr);
				}
				if (_hazardpaint.bonedrought != _board.bonedrought(index))
				{
					Change change(Change::Type::BONEDROUGHT, _paintdesc);
					change.xBonedrought(_hazardpaint.bonedrought);
					_board.enact(change);
					_level.enact(change, nullptr);
				}
				if (_hazardpaint.death != _board.death(index))
				{
					Change change(Change::Type::DEATH, _paintdesc);
					change.xDeath(_hazardpaint.death);
					_board.enact(change);
					_level.enact(change, nullptr);
				}
			}
			break;

			default: break;
		}
	}
	else if (Input::get()->isKeyHeld(SDL_SCANCODE_RMB) && !index.edge())
	{
		switch (_paintdesc.type)
		{
			case Descriptor::Type::TILE:
			{
				const TileToken& existing = _board.tile(index);

				_tilepaint = existing;
			}
			break;

			case Descriptor::Type::GROUND:
			case Descriptor::Type::AIR:
			{
				const UnitToken& existing = _board.unit(index, _paintdesc.type);

				_unitpaint = existing;
			}
			break;

			case Descriptor::Type::CELL:
			{
				_hazardpaint.gas = _board.gas(index);
				_hazardpaint.radiation = _board.radiation(index);
				_hazardpaint.death = _board.death(index);
				_hazardpaint.firestorm = _board.firestorm(index);
				_hazardpaint.frostbite = _board.frostbite(index);
				_hazardpaint.bonedrought = _board.bonedrought(index);
			}
			break;

			default: break;
		}
	}

	switch (_paintdesc.type)
	{
		case Descriptor::Type::TILE:
		{
			if (ImGui::Begin("Paint Brush"))
			{
				ImGui::Separator();

				int tp = static_cast<int>(_tilepaint.type);
				for (int i = 1; i <= _bible.tiletype_max(); i++)
				{
					TileType type = static_cast<TileType>(i);
					const char* txt = ::stringify(_bible.typeword(type));
					ImGui::RadioButton(txt, &tp, i);
				}
				TileType newtype = static_cast<TileType>(tp);
				if (newtype != _tilepaint.type)
				{
					_tilepaint.type = newtype;
					if (!_bible.tileOwnable(_tilepaint.type))
					{
						_tilepaint.owner = Player::NONE;
					}
					_tilepaint.stacks = _bible.tileStacksBuilt(newtype);
					_tilepaint.power = _bible.tilePowerBuilt(newtype);
				}

				ImGui::Separator();

				if (_bible.tileOwnable(_tilepaint.type))
				{
					int pl = static_cast<int>(_tilepaint.owner);
					for (Player player : allPlayers)
					{
						const char* txt = ::stringify(player);
						ImGui::RadioButton(txt, &pl, static_cast<int>(player));
					}
					_tilepaint.owner = static_cast<Player>(pl);
				}
				else
				{
					_tilepaint.owner = Player::NONE;
				}

				ImGui::Separator();

				if (_bible.tileStacksMax(_tilepaint.type) > 0)
				{
					int i = _tilepaint.stacks;
					ImGui::SliderInt("Stacks", &i, 1,
						_bible.tileStacksMax(_tilepaint.type));
					_tilepaint.stacks = i;
				}
				else
				{
					_tilepaint.stacks = 0;
				}

				if (_bible.tilePowerMax(_tilepaint.type) > 0)
				{
					int i = std::min(_tilepaint.stacks, _tilepaint.power);
					int max = std::min(_tilepaint.stacks,
						_bible.tilePowerMax(_tilepaint.type));
					ImGui::SliderInt("Power", &i, 0, max);
					_tilepaint.power = i;
				}
				else
				{
					_tilepaint.power = 0;
				}
			}
			ImGui::End();
		}
		break;

		case Descriptor::Type::GROUND:
		case Descriptor::Type::AIR:
		{
			bool air = (_paintdesc.type == Descriptor::Type::AIR);

			if (ImGui::Begin("Paint Brush"))
			{
				ImGui::Separator();

				if (_unitpaint.type != UnitType::NONE
					&& _bible.unitAir(_unitpaint.type) != air)
				{
					_unitpaint.type = UnitType::NONE;
				}
				int tp = static_cast<int>(_unitpaint.type);
				for (int i = 0; i <= _bible.unittype_max(); i++)
				{
					UnitType type = static_cast<UnitType>(i);
					if (type == UnitType::NONE || _bible.unitAir(type) == air)
					{
						const char* txt = ::stringify(_bible.typeword(type));
						ImGui::RadioButton(txt, &tp, i);
					}
				}
				UnitType newtype = static_cast<UnitType>(tp);
				if (newtype != _unitpaint.type)
				{
					_unitpaint.type = newtype;
					_unitpaint.stacks = (newtype != UnitType::NONE) ? 1 : 0;
				}

				ImGui::Separator();

				if (_unitpaint.type != UnitType::NONE)
				{
					int pl = static_cast<int>(_unitpaint.owner);
					for (Player player : allPlayers)
					{
						const char* txt = ::stringify(player);
						if (player == Player::NONE)
						{
							// This looks nicer but it actually fixes a bug
							// because the imgui button id "none" is already
							// used for unit types.
							txt = "neutral";
						}
						ImGui::RadioButton(txt, &pl, static_cast<int>(player));
					}
					_unitpaint.owner = static_cast<Player>(pl);
				}

				ImGui::Separator();

				if (_bible.unitStacksMax(_unitpaint.type) > 0)
				{
					int i = _unitpaint.stacks;
					ImGui::SliderInt("Stacks", &i, 1,
						_bible.unitStacksMax(_unitpaint.type));
					_unitpaint.stacks = i;
				}
			}
			ImGui::End();
		}
		break;

		case Descriptor::Type::CELL:
		{
			if (ImGui::Begin("Paint Brush"))
			{
				ImGui::Separator();

				if (_bible.gasMin() < _bible.gasMax())
				{
					int i = (int) _hazardpaint.gas;
					ImGui::SliderInt("Gas", &i,
						_bible.gasMin(), _bible.gasMax());
					_hazardpaint.gas = (uint8_t) i;
				}
				if (_bible.radiationMin() < _bible.radiationMax())
				{
					int i = (int) _hazardpaint.radiation;
					ImGui::SliderInt("Radiation", &i,
						_bible.radiationMin(), _bible.radiationMax());
					_hazardpaint.radiation = (uint8_t) i;
				}
				ImGui::Checkbox("Frostbite", &_hazardpaint.frostbite);
				ImGui::Checkbox("Firestorm", &_hazardpaint.firestorm);
				ImGui::Checkbox("Bonedrought", &_hazardpaint.bonedrought);
				ImGui::Checkbox("Death", &_hazardpaint.death);
			}
			ImGui::End();
		}
		break;

		default: break;
	}
}

void MapEditor::updateParameters()
{
	if (ImGui::Begin("Parameters", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Map Pool: %s", ::stringify(_pooltype));
		if (!isValidPoolTypeForWorkshop(_pooltype))
		{
			ImVec4 invalidcolor(1.0f, 0.4f, 0.2f, 1.0f);
			ImGui::TextColored(invalidcolor,
				"Invalid Map Pool!");
		}
		ImGui::Text("Ruleset: %s", _bible.name().c_str());
		if (ImGui::Button("Change"))
		{
			changeRuleset();
		}
		ImGui::SameLine();
		if (ImGui::Button("Edit"))
		{
			switchToRulesetEditor();
		}
		{
			bool isChallenge = (bool) _challenge;
			ImGui::Checkbox("Challenge", &isChallenge);
			if (isChallenge && !_challenge)
			{
				_challenge.reset(new ChallengeEditData());
			}
			else if (!isChallenge && _challenge)
			{
				_challenge.reset();
			}
			if (_challenge)
			{
				ImGui::SameLine();
				bool wasAdvanced = _challenge->advanced;
				ImGui::Checkbox("Advanced", &(_challenge->advanced));
				if (!_challenge->advanced && wasAdvanced)
				{
					_challenge->stars = 1;
					_playercount = 2;
				}
			}
		}
		ImGui::Separator();

		if (_challenge && !_challenge->advanced)
		{
			ImGui::SliderInt("Players", &_playercount, 2, 2);
		}
		else
		{
			ImGui::SliderInt("Players", &_playercount, 2, PLAYER_MAX);
		}
		ImGui::Separator();

		bool regenerateForests = false;
		bool repopulateTiles = false;
		{
			int oldx = std::round(5 * _skinner.climateModifier);
			int newx = oldx;
			ImGui::SliderInt("Climate", &newx, -5, 5);
			if (newx != oldx)
			{
				_skinner.climateModifier = 0.2f * newx;
				repopulateTiles = true;
			}
		}
#if EDITOR_DEPRECATED_ENABLED
		{
			int oldx = std::round(5 * _skinner.raininess);
			int newx = oldx;
			ImGui::SliderInt("Raininess", &newx, 0, 5);
			if (newx != oldx)
			{
				_skinner.raininess = 0.2f * newx;
				repopulateTiles = true;
			}
		}
#endif
		{
			ImGui::Text("Moderate Tree Types:");
			for (auto iter = _skinner.treetypes.begin();
				iter != _skinner.treetypes.end(); /**/)
			{
				TreeType treetype = *iter;
				std::string label = " - ";
				label += ::stringify(treetype);
				bool selected = true;
				ImGui::Selectable(label.c_str(), &selected);
				if (!selected)
				{
					iter = _skinner.treetypes.erase(iter);
					regenerateForests = true;
					repopulateTiles = true;
				}
				else ++iter;
			}
			// Skip NONE.
			for (size_t i = 1; i < TREETYPE_SIZE; i++)
			{
				TreeType treetype = (TreeType) i;
				if (std::find(_skinner.treetypes.begin(),
					_skinner.treetypes.end(),
					treetype) != _skinner.treetypes.end())
				{
					continue;
				}

				std::string label = " + ";
				label += ::stringify(treetype);
				bool selected = false;
				if (_skinner.treetypes.empty() && treetype == TreeType::OAK)
				{
					selected = true;
				}
				ImGui::Selectable(label.c_str(), &selected);
				if (selected)
				{
					_skinner.treetypes.emplace_back(treetype);
					regenerateForests = true;
					repopulateTiles = true;
				}
			}
		}
		if (regenerateForests)
		{
			_level.generateForests();
		}
		if (repopulateTiles)
		{
			for (Square& square : _level)
			{
				if (square.tile())
				{
					square.tile().populate(nullptr);
					square.cleanup();
				}
			}
			_level.generateBorders();
			_level.setBorderLight(1, 0);
		}
	}
	ImGui::End();
}

void ChallengeEditData::update()
{
	if (ImGui::Begin("Challenge", nullptr,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::InputText("Challenge name", name.data(), name.size());
		if (advanced)
		{
			ImGui::SliderInt("Max stars", &stars, 1, 3);
		}

		ImGui::Separator();
		ImGui::Text("Mission Briefing");
		ImGui::InputTextWithHint("Greeting", "Greetings, Commander!",
			greeting.data(), greeting.size());
		ImGui::InputTextMultiline("Description",
			description.data(), description.size());
		ImGui::InputTextMultiline("Objective",
			objective.data(), objective.size());
		if (advanced)
		{
			ImGui::InputText("First star",
				one.data(), one.size());
			if (stars >= 2)
			{
				ImGui::InputText("Second star",
					two.data(), two.size());
			}
			if (stars >= 3)
			{
				ImGui::InputText("Third star",
					three.data(), three.size());
			}
		}
		else
		{
			ImGui::LabelText("First star", "Defeat the enemy.");
		}
		ImGui::InputTextWithHint("Sendoff", "Good luck!",
			sendoff.data(), sendoff.size());
	}
	ImGui::End();
}

void MapEditor::updateGlobals()
{
	int totalhumidity = 0;
	int totalaccessible = 0;
	int totalwalkable = 0;
	int totalgrassy = 0;
	int count = (int) _rows * _cols;
	for (Cell index : _board)
	{
		totalhumidity += _board.humidity(index);
		if (_bible.tileAccessible(_board.tile(index).type)) totalaccessible++;
		if (_bible.tileWalkable(_board.tile(index).type)) totalwalkable++;
		if (_bible.tileGrassy(_board.tile(index).type)) totalgrassy++;
	}

	int averagehumidity = (totalhumidity + count / 2) / count;

	if (ImGui::Begin("Info", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
#if EDITOR_DEPRECATED_ENABLED
		ImGui::PushItemWidth(80);

		int h = averagehumidity;
		ImGui::InputInt("Average humidity", &h, 1, 0, 4);
		if (h != averagehumidity)
		{
			for (int r = 0; r < _rows; r++)
			{
				for (int c = 0; c < _cols; c++)
				{
					Change change(Change::Type::HUMIDITY, Descriptor::cell(Position(r, c)));
					change.xHumidity(h - averagehumidity);
					_board.enact(change);
					_level.enact(change, nullptr);
				}
			}
		}

		if (h != averagehumidity
			&& _pooltype != PoolType::DIORAMA)
		{
			ChangeSet changes = Automaton::adjustMarkersOnBoard(_bible, _board);

			for (const Change& change : changes.get(Player::OBSERVER))
			{
				_level.enact(change, nullptr);
			}
		}

		if (_pooltype == PoolType::DIORAMA
			&& ImGui::Button("Recalculate humidity"))
		{
			ChangeSet changes = Automaton::setupMarkersOnBoard(_bible, _board);
			for (const Change& change : changes.get(Player::OBSERVER))
			{
				_level.enact(change, nullptr);
			}
		}

		ImGui::PopItemWidth();
#else
		ImGui::Text("Average humidity: %d", averagehumidity);
#endif

		ImGui::Separator();
		ImGui::Text("Rows: %d", _board.rows());
		if (_board.rows() == Position::MAX_ROWS) ImGui::Text("(max reached)");
		else if (ImGui::Button("Expand Top")) expandTop();
		if (_board.rows() == Position::MAX_ROWS) ImGui::Text("(max reached)");
		else if (ImGui::Button("Expand Bottom")) expandBottom();
		if (ImGui::Button("Crop Top")) cropTop();
		if (ImGui::Button("Crop Bottom")) cropBottom();
		ImGui::Text("Columns: %d", _board.cols());
		if (_board.cols() == Position::MAX_COLS) ImGui::Text("(max reached)");
		else if (ImGui::Button("Expand Left")) expandLeft();
		if (_board.cols() == Position::MAX_COLS) ImGui::Text("(max reached)");
		else if (ImGui::Button("Expand Right")) expandRight();
		if (ImGui::Button("Crop Left")) cropLeft();
		if (ImGui::Button("Crop Right")) cropRight();

		ImGui::Separator();
		ImGui::Text("Number of accessible tiles: %d", totalaccessible);
		ImGui::Text("Number of walkable tiles: %d", totalwalkable);
		ImGui::Text("Number of grassy tiles: %d", totalgrassy);
		ImGui::Text("Number of tiles for chaos: %d", 125 * _board.mass());
	}
	ImGui::End();
}

void MapEditor::updateRulesetEditor()
{
	if (ImGui::Begin("Ruleset Editor", &_showRulesetEditor,
			ImGuiWindowFlags_AlwaysAutoResize
			| (_rulesetunsaved ? ImGuiWindowFlags_UnsavedDocument : 0)))
	{
		ImGui::Text("Ruleset: %s", _rulesetname.c_str());
		if (ImGui::Button("Load"))
		{
			changeRuleset();
		}
		if (_rulesetbuffer[0] != '\0' && _rulesetvalid)
		{
			if (System::isFile(Locator::rulesetAuthoredFilename(
					_rulesetname)))
			{
				ImGui::SameLine();
				if (ImGui::Button("Save"))
				{
					_rulesetsaving = true;
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Save As"))
			{
				_activepopup = Popup::SAVE_RULESET;
			}
		}

		std::unique_ptr<Bible> parsed;
		bool loaded = false;
		if (_rulesetbuffer[0] == '\0' && _rulesetname == _bible.name())
		{
			Json::Value json = _bible.toJson();
			std::stringstream strm;
			strm << json;
			std::string str = strm.str();
			if (str.size() >= _rulesetbuffer.size())
			{
				str = "{}";
			}
			strncpy(_rulesetbuffer.data(), str.c_str(),
				_rulesetbuffer.size() - 1);
			loaded = true;
		}
		bool updated = ImGui::InputTextMultiline("Json",
			_rulesetbuffer.data(), _rulesetbuffer.size(),
			ImVec2(600.0f, 500.0f));
		if (_rulesetbuffer[0] == '\0')
		{
			_rulesetvalid = false;
			_rulesetunsaved = false;
			_rulesetsaving = false;
			_ruleseterrormessage = "Empty";
		}
		else if (loaded || updated || _rulesetsaving)
		{
			std::string str = _rulesetbuffer.data();
			Json::Reader reader;
			Json::Value json;
			_rulesetvalid = reader.parse(str, json);
			if (!_rulesetvalid)
			{
				// Do not show actual error message because it contains
				// line numbers, but this is not a text editor.
				_ruleseterrormessage = "Invalid JSON";
			}

			if (_rulesetvalid)
			{
				try
				{
					parsed.reset(new Bible(_rulesetname, std::move(json)));
				}
				catch (const ParseError& error)
				{
					_rulesetvalid = false;
					_ruleseterrormessage = error.what();
				}
				catch (const Json::Exception& error)
				{
					_rulesetvalid = false;
					_ruleseterrormessage = error.what();
				}
			}
		}

		_rulesetunsaved |= updated;

		if (_rulesetsaving)
		{
			_rulesetsaving = false;
			if (_rulesetvalid && parsed)
			{
				// Convert the parsed ruleset back into JSON. This strips
				// any unused properties and normalizes the output a bit.
				Json::Value json = parsed->toJson();
				std::stringstream strm;
				strm << json;
				std::string fname = Locator::rulesetAuthoredFilename(
					_rulesetname);
				System::touchFile(fname);
				std::ofstream file = System::ofstream(fname);
				if (file)
				{
					file << strm.rdbuf();
					_rulesetunsaved = false;
				}
				else
				{
					_rulesetvalid = false;
					_ruleseterrormessage = "Failed to write to file";
				}
			}
		}

		if (_rulesetvalid)
		{
			ImVec4 validcolor(0.1f, 0.8f, 0.4f, 1.0f);
			ImGui::TextColored(validcolor, "Valid");
			ImGui::TextDisabled("No errors.");
		}
		else
		{
			ImVec4 invalidcolor(1.0f, 0.4f, 0.2f, 1.0f);
			ImGui::TextColored(invalidcolor, "Error");
			ImGui::Text("%s", _ruleseterrormessage.c_str());
		}
	}
	ImGui::End();
}

void MapEditor::generateBiomes()
{
	if (_pooltype != PoolType::DIORAMA)
	{
		ChangeSet changes = Automaton::setupMarkersOnBoard(_bible, _board);
		for (const Change& change : changes.get(Player::OBSERVER))
		{
			_level.enact(change, nullptr);
		}
	}

	_level.generateForests();
	_level.generateBorders();
}

void MapEditor::clear()
{
	_unsavedCached = unsaved();
	_activepopup = Popup::NEW;
}

void MapEditor::onNewDimensions(int cols, int rows)
{
	_cols = cols;
	_rows = rows;
	_board.clear(_cols, _rows);
	_level.clear(_cols, _rows);

	_pooltype = PoolType::MULTIPLAYER;
	_bible = Library::getBible(Library::nameCurrentBible());
	_rulesetname = "";

	loadEmpty();

	_mapname = "";
	_showRulesetEditor = false;
	_owner.closeWorkshop();
}

void MapEditor::expandTop()
{
	ChangeSet changes;
	store(Position(/*row=*/-1, /*col=*/0), changes);

	_rows += 1;
	_board.clear(_cols, _rows);
	_level.clear(_cols, _rows);

	restore(changes);
}

void MapEditor::expandLeft()
{
	ChangeSet changes;
	store(Position(/*row=*/0, /*col=*/-1), changes);

	_cols += 1;
	_board.clear(_cols, _rows);
	_level.clear(_cols, _rows);

	restore(changes);
}

void MapEditor::expandRight()
{
	ChangeSet changes;
	store(Position(/*row=*/0, /*col=*/0), changes);

	_cols += 1;
	_board.clear(_cols, _rows);
	_level.clear(_cols, _rows);

	restore(changes);
}

void MapEditor::expandBottom()
{
	ChangeSet changes;
	store(Position(/*row=*/0, /*col=*/0), changes);

	_rows += 1;
	_board.clear(_cols, _rows);
	_level.clear(_cols, _rows);

	restore(changes);
}

void MapEditor::cropTop()
{
	ChangeSet changes;
	store(Position(/*row=*/+1, /*col=*/0), changes);

	_rows -= 1;
	_board.clear(_cols, _rows);
	_level.clear(_cols, _rows);

	restore(changes);
}

void MapEditor::cropLeft()
{
	ChangeSet changes;
	store(Position(/*row=*/0, /*col=*/+1), changes);

	_cols -= 1;
	_board.clear(_cols, _rows);
	_level.clear(_cols, _rows);

	restore(changes);
}

void MapEditor::cropRight()
{
	ChangeSet changes;
	store(Position(/*row=*/0, /*col=*/0), changes);

	_cols -= 1;
	_board.clear(_cols, _rows);
	_level.clear(_cols, _rows);

	restore(changes);
}

void MapEditor::cropBottom()
{
	ChangeSet changes;
	store(Position(/*row=*/0, /*col=*/0), changes);

	_rows -= 1;
	_board.clear(_cols, _rows);
	_level.clear(_cols, _rows);

	restore(changes);
}

void MapEditor::store(const Position& topleft, ChangeSet& changes)
{
	for (Cell index : _board)
	{
		Position position = index.pos();
		position.row -= topleft.row;
		position.col -= topleft.col;

		changes.push(Change(Change::Type::REVEAL,
				Descriptor::cell(position),
				_board.tile(index),
				false, false, false, false, false,
				0, 0, 0, 0, 0),
			Vision::none());

		if (_board.ground(index))
		{
			changes.push(Change(Change::Type::ENTERED,
					Descriptor::ground(position),
					_board.ground(index)),
				Vision::none());
		}

		if (_board.air(index))
		{
			changes.push(Change(Change::Type::ENTERED,
					Descriptor::air(position),
					_board.air(index)),
				Vision::none());
		}
	}
}

void MapEditor::restore(const ChangeSet& changes)
{
	for (const Change& change : changes.get(Player::OBSERVER))
	{
		if (change.subject.type != Descriptor::Type::NONE
			&& !_board.cell(change.subject.position).valid())
		{
			continue;
		}

		_board.enact(change);
		_level.enact(change, nullptr);
	}

	TileType watertype = _bible.tiletype("water");
	if (watertype == TileType::NONE)
	{
		LOGE << "Default tiletypes changed";
		DEBUG_ASSERT(false);
	}

	for (Cell index : _board)
	{
		if (!_board.tile(index))
		{
			TileToken newtoken;
			newtoken.type = watertype;
			Change change(Change::Type::REVEAL,
				Descriptor::cell(index.pos()),
				newtoken, false,
				false, false, false, false,
				0, 0, 0, 0, 0);
			_board.enact(change);
			_level.enact(change, nullptr);
		}
	}

	loaded();
}

void MapEditor::loadEmpty()
{
	TileType watertype = _bible.tiletype("water");
	TileType grasstype = _bible.tiletype("grass");
	if (watertype == TileType::NONE || grasstype == TileType::NONE)
	{
		LOGE << "Default tiletypes changed";
		DEBUG_ASSERT(false);
	}

	for (Cell index : _board)
	{
		Position position = index.pos();
		int r = position.row;
		int c = position.col;
		TileToken newtoken;
		if (r < 2 || c < 2 || r >= _rows - 2 || c >= _cols - 2)
		{
			newtoken.type = watertype;
		}
		else
		{
			newtoken.type = grasstype;
		}
		Change change(Change::Type::REVEAL,
			Descriptor::cell(position),
			newtoken, false,
			false, false, false, false,
			0, 0, 0, 0, 0);
		_board.enact(change);
		_level.enact(change, nullptr);
	}

	if (_skinner.treetypes.empty())
	{
		_skinner.treetypes.emplace_back(TreeType::OAK);
	}

	_challenge.reset();

	loaded();

	saved();
}

void MapEditor::load()
{
	loadFromFile(_mapname);
}

void MapEditor::loadFrom()
{
	_activepopup = Popup::OPEN_FILE;
	_cachedMapNames = Map::listAuthored();
}

void MapEditor::loadCopyFrom()
{
	_activepopup = Popup::COPY_FILE;
	_cachedMapNames = Map::listAuthored();
}

void MapEditor::onOpenMapName(const std::string& mapname)
{
	_mapname = mapname;
	loadFromFile(mapname);
	_showRulesetEditor = false;
	_owner.closeWorkshop();
}

void MapEditor::onCopyMapName(const std::string& mapname)
{
	_mapname = "";
	loadFromFile(mapname);
	_showRulesetEditor = false;
	_owner.closeWorkshop();
}

void MapEditor::loadFromFile(const std::string& mapname)
{
	Json::Reader reader;
	Json::Value metadata;
	std::string line;

	std::string fname = Map::readOnlyFilename(mapname);
	LOGI << "Loading " << mapname << " from " << fname;

	std::ifstream file = System::ifstream(fname);
	if (!std::getline(file, line) || !reader.parse(line, metadata)
		|| !metadata.isObject())
	{
		// Try old style.
		file.close();
		file.open(fname);
		if (!reader.parse(file, metadata) || !metadata.isObject())
		{
			LOGE << "Failed to load map '" << mapname << "' (" << fname << "): "
				<< reader.getFormattedErrorMessages();
			onLoadFailure();
			return;
		}
	}

	// Read the version of the editor that the map was last saved with.
	Version myversion = Version::current();
	Version editorversion;
	if (metadata["editor-version"].isString())
	{
		editorversion = Version(metadata["editor-version"]);
	}
	else editorversion = Version::prehistoric();

	// Check compatibility.
	if (editorversion.major > myversion.major
		&& !editorversion.isDevelopment()
		&& !myversion.isDevelopment())
	{
		LOGE << "version mismatch while parsing map";
		onLoadFailure();
		return;
	}

	// Read pooltype.
	bool validpooltype = true;
	if (metadata["pool"].isString())
	{
		_pooltype = parsePoolType(metadata["pool"].asString());
	}
	else _pooltype = PoolType::NONE;

	// Read the name of the ruleset that the map was last saved with.
	std::string rulesetname;
	if (metadata["ruleset"].isString())
	{
		rulesetname = metadata["ruleset"].asString();
		if (rulesetname.find("255.255.255") != std::string::npos)
		{
			rulesetname = Library::nameCurrentBible();
		}
		else if (rulesetname == "custom")
		{
			// The ruleset 'challenge_trample' was named 'custom' by accident.
			if (mapname.compare(0, 10, "challenge_") == 0)
			{
				LOGI << "Overriding rulesetname with '" << mapname << "'";
				rulesetname = mapname;
			}
		}
	}
	else if (metadata["bible-version"].isString())
	{
		// Old map where every ruleset used was the master ruleset.
		Version bibleversion = Version(metadata["bible-version"]);
		if (bibleversion.isDevelopment())
		{
			rulesetname = Library::nameCurrentBible();
		}
		else rulesetname = Library::nameCompatibleBible(bibleversion);
	}
	else
	{
		rulesetname = Library::nameCompatibleBible(Version::prehistoric());
	}

	// Look for a bible with that version.
	if (!Library::existsBible(rulesetname))
	{
		// Backwards compatibility: retry with a compatible ruleset.
		rulesetname = Library::nameCompatibleBible(rulesetname);

		if (!Library::existsBible(rulesetname))
		{
			LOGE << "missing bible '" << rulesetname << "'"
				" while parsing map";
			onLoadFailure();
			return;
		}
	}

	// Get the bible version.
	Version bibleversion = Library::getBible(rulesetname).version();

	// Check compatibility.
	if ((bibleversion.major > myversion.major
			|| (bibleversion.minor == myversion.minor
				&& bibleversion.minor > myversion.minor))
		&& !bibleversion.isDevelopment()
		&& !myversion.isDevelopment())
	{
		LOGE << "version mismatch while parsing map";
		onLoadFailure();
		return;
	}

	// Load the bible. This works because the Level and the Boards have a
	// reference to _bible, which remains valid when we change its data.
	if (_pooltype == PoolType::MULTIPLAYER)
	{
		// For multiplayer maps, always use the most recent ruleset.
		_bible = Library::getBible(Library::nameCurrentBible());
	}
	else
	{
		// For custom maps, load the named ruleset.
		_bible = Library::getBible(rulesetname);
		// For authored rulesets, load it directly to avoid caching.
		if (System::isFile(Locator::rulesetAuthoredFilename(rulesetname)))
		{
			std::ifstream rulesetfile = System::ifstream(
				Locator::rulesetAuthoredFilename(rulesetname));
			Json::Value rulesetjson;
			if (reader.parse(rulesetfile, rulesetjson))
			{
				try
				{
					_bible = Bible(rulesetname, std::move(rulesetjson));
				}
				catch (const ParseError& error)
				{
					LOGW << "Failed to parse authored ruleset: "
						<< error.what();
				}
				catch (const Json::Exception& error)
				{
					LOGW << "Failed to parse authored ruleset: "
						<< error.what();
				}
			}
			else
			{
				LOGW << "Failed to open authored ruleset from file "
					<< Locator::rulesetAuthoredFilename(rulesetname);
			}
		}
	}

	// Read the board.
	try
	{
		_board.load(mapname);
	}
	catch (const ParseError& error)
	{
		if (_pooltype == PoolType::MULTIPLAYER)
		{
			// If loading the board fails, it might be because the map features
			// tiles or units that no longer exist in the newest ruleset.
			// The user must manually set the pooltype again to resolve this.
			LOGW << "Resetting pooltype due to parse error: " << error.what();
			validpooltype = false;
			LOGI << "Trying again with ruleset '" << rulesetname << "'";
			_bible = Library::getBible(rulesetname);

			// Try again. If this also fails, there is actually something wrong.
			try
			{
				_board.load(mapname);
			}
			catch (const ParseError& seconderror)
			{
				LOGE << "Failed to load map: " << seconderror.what();
				onLoadFailure();
				return;
			}
		}
		else
		{
			LOGE << "Failed to load map: " << error.what();
			onLoadFailure();
			return;
		}
	}

	// Get dimensions.
	_cols = metadata["cols"].asInt();
	_rows = metadata["rows"].asInt();

	// Get supported player count.
	_playercount = (!metadata["playercount"].isNull()) ?
		metadata["playercount"].asInt() : 2;

	// Get skinner data.
	_skinner.clear();
	_skinner.load(metadata);

	// Get challenge metadata, if any.
	if (metadata["challenge"].isObject())
	{
		_challenge.reset(new ChallengeEditData());
		_challenge->load(metadata);
		if (_playercount != 2)
		{
			_challenge->advanced = true;
		}
	}
	else _challenge.reset();

	// Mimick the board with the level.
	_level.clear(_cols, _rows);

	for (Cell index : _board)
	{
		Position position = index.pos();
		const Square* oldsquare = _level.cell(position);

		{
			Change change(Change::Type::REVEAL,
				Descriptor::tile(position),
				_board.tile(index),
				_board.snow(index),
				_board.frostbite(index),
				_board.firestorm(index),
				_board.bonedrought(index),
				_board.death(index),
				_board.gas(index),
				_board.radiation(index),
				_board.temperature(index),
				_board.humidity(index),
				_board.chaos(index));
			_level.enact(change, nullptr);
		}

		if (oldsquare->ground())
		{
			Change change(Change::Type::EXITED,
				Descriptor::ground(position));
			_level.enact(change, nullptr);
		}
		if (_board.ground(index))
		{
			Change change(Change::Type::ENTERED,
				Descriptor::ground(position),
				_board.ground(index));
			_level.enact(change, nullptr);
		}

		if (oldsquare->air())
		{
			Change change(Change::Type::EXITED,
				Descriptor::air(position));
			_level.enact(change, nullptr);
		}
		if (_board.air(index))
		{
			Change change(Change::Type::ENTERED,
				Descriptor::air(position),
				_board.air(index));
			_level.enact(change, nullptr);
		}
	}

	loaded();

	saved();

	if (!validpooltype)
	{
		// If loading the board failed because the map features tiles or units
		// that no longer exist in the newest ruleset, then the user must
		// manually set the pooltype again to resolve this.
		_pooltype = PoolType::NONE;
	}
}

void MapEditor::loaded()
{
	for (Square& square : _level)
	{
		square.setLight(1, 1);
	}

	for (Cell index : _board)
	{
		_board.vision(index).add(Player::OBSERVER);
	}

	generateBiomes();

	for (Square& square : _level)
	{
		if (square.tile())
		{
			square.tile().populate(nullptr);
			square.cleanup();
		}
	}

	_level.setBorderLight(1, 0);

	_camerafocus->set(_level.centerPoint());
	_camerafocus->load(_level.topleftPoint(), _level.bottomrightPoint());
}

void MapEditor::onLoadFailure()
{
	_activepopup = Popup::ERROR;
}

void MapEditor::save()
{
	saveToFile(_mapname);
}

void MapEditor::saveAs()
{
	_activepopup = Popup::SAVE_AS;
}

void MapEditor::saveCopyAs()
{
	_activepopup = Popup::SAVE_COPY_AS;
}

void MapEditor::onSaveAsMapName(const std::string& mapname)
{
	_mapname = mapname;
	saveToFile(mapname);
	_showRulesetEditor = false;
	_owner.closeWorkshop();
}

void MapEditor::onSaveCopyAsMapName(const std::string& mapname)
{
	saveToFile(mapname);
}

void MapEditor::saveToFile(const std::string& mapname)
{
	std::string fname = Map::authoredFilename(mapname);
	LOGI << "Saving " << mapname << " to " << fname;

	System::touchFile(fname);
	std::ofstream file = System::ofstream(fname);

	Json::Value metadata;
	metadata["editor-version"] = Version::current().toString();
	if (_pooltype != PoolType::NONE)
	{
		metadata["pool"] = ::stringify(_pooltype);
	}
	metadata["ruleset"] = _bible.name();
	metadata["cols"] = _cols;
	metadata["rows"] = _rows;
	metadata["playercount"] = _playercount;
	_skinner.store(metadata);
	if (_challenge) _challenge->store(metadata);
	file << Writer::write(metadata) << std::endl;
	file << TypeEncoder(&_bible);

	for (Cell index : _board)
	{
		file << "{";
		bool empty = true;
		if (_board.tile(index))
		{
			if (empty) empty = false;
			else file << ",";
			file << "\"tile\":" << _board.tile(index);
		}
		if (_board.ground(index))
		{
			if (empty) empty = false;
			else file << ",";
			file << "\"ground\":" << _board.ground(index);
		}
		if (_board.air(index))
		{
			if (empty) empty = false;
			else file << ",";
			file << "\"air\":" << _board.air(index);
		}
		if (_pooltype == PoolType::DIORAMA && !empty)
		{
			if (_board.temperature(index) != 0)
			{
				file << ",\"temperature\":" << (int) _board.temperature(index);
			}
			if (_board.humidity(index) != 0)
			{
				file << ",\"humidity\":" << (int) _board.humidity(index);
			}
			if (_board.chaos(index) != 0)
			{
				file << ",\"chaos\":" << (int) _board.chaos(index);
			}
			if (_board.gas(index) != 0)
			{
				file << ",\"gas\":" << (int) _board.gas(index);
			}
			if (_board.radiation(index) != 0)
			{
				file << ",\"radiation\":" << (int) _board.radiation(index);
			}
			if (_board.snow(index))
			{
				file << ",\"snow\":true";
			}
			if (_board.frostbite(index))
			{
				file << ",\"frostbite\":true";
			}
			if (_board.firestorm(index))
			{
				file << ",\"firestorm\":true";
			}
			if (_board.bonedrought(index))
			{
				file << ",\"bonedrought\":true";
			}
			if (_board.death(index))
			{
				file << ",\"death\":true";
			}
		}
		file << "}" << std::endl;
	}

	saved();
}

void MapEditor::saved()
{
	_savedpooltype = _pooltype;

	if (_saved.cols() != _board.cols() || _saved.rows() != _board.rows())
	{
		_saved.clear(_board.cols(), _board.rows());
	}

	for (Cell index : _board)
	{
		_saved.tile(index) = _board.tile(index);
		_saved.ground(index) = _board.ground(index);
		_saved.air(index) = _board.air(index);
		_saved.temperature(index) = _board.temperature(index);
		_saved.humidity(index) = _board.humidity(index);
		_saved.chaos(index) = _board.chaos(index);
		_saved.gas(index) = _board.gas(index);
		_saved.radiation(index) = _board.radiation(index);
		_saved.snow(index) = _board.snow(index);
		_saved.frostbite(index) = _board.frostbite(index);
		_saved.firestorm(index) = _board.firestorm(index);
		_saved.bonedrought(index) = _board.bonedrought(index);
		_saved.death(index) = _board.death(index);
	}

	_unsavedCached = false;
}

bool MapEditor::unsaved()
{
	if (_savedpooltype != _pooltype) return true;

	if (_saved.cols() != _board.cols() || _saved.rows() != _board.rows())
	{
		return true;
	}

	for (Cell index : _board)
	{
		if (_saved.tile(index) != _board.tile(index)) return true;
		if (_saved.ground(index) != _board.ground(index)) return true;
		if (_saved.air(index) != _board.air(index)) return true;
		if (_saved.temperature(index) != _board.temperature(index)) return true;
		if (_saved.humidity(index) != _board.humidity(index)) return true;
		if (_saved.chaos(index) != _board.chaos(index)) return true;
		if (_saved.gas(index) != _board.gas(index)) return true;
		if (_saved.radiation(index) != _board.radiation(index)) return true;
		if (_saved.snow(index) != _board.snow(index)) return true;
		if (_saved.frostbite(index) != _board.frostbite(index)) return true;
		if (_saved.firestorm(index) != _board.firestorm(index)) return true;
		if (_saved.bonedrought(index) != _board.bonedrought(index)) return true;
		if (_saved.death(index) != _board.death(index)) return true;
	}

	return false;
}

void MapEditor::changeRuleset()
{
	_activepopup = Popup::CHANGE_RULESET;
	_cachedRulesetNames = Locator::listAuthoredRulesets();
}

void MapEditor::onChangeRuleset(const PoolType& pooltype,
		const std::string& rulesetname)
{
	if (!Library::existsBible(rulesetname))
	{
		LOGE << "Unknown ruleset '" + rulesetname + "'";
		return;
	}

	Bible& oldbible = _bible;
	Bible newbible = Library::getBible(rulesetname);
	if (System::isFile(Locator::rulesetAuthoredFilename(rulesetname)))
	{
		std::string fname = Locator::rulesetAuthoredFilename(rulesetname);
		std::ifstream file = System::ifstream(fname);
		Json::Reader reader;
		Json::Value json;
		if (reader.parse(file, json))
		{
			try
			{
				newbible = Bible(rulesetname, std::move(json));
			}
			catch (const ParseError& error)
			{
				LOGW << "Failed to parse authored ruleset: " << error.what();
			}
			catch (const Json::Exception& error)
			{
				LOGW << "Failed to parse authored ruleset: " << error.what();
			}
		}
		else
		{
			LOGW << "Failed to open authored ruleset from file " << fname;
		}
	}

	try
	{
		for (Cell index : _board)
		{
			if (_board.tile(index))
			{
				std::stringstream strm;
				strm << TypeEncoder(&oldbible);
				strm << _board.tile(index).type;
				TypeWord typeword = parseTypeWord(strm.str());
				TileType newtype = newbible.tiletype(::stringify(typeword));
				_board.tile(index).type = newtype;
			}
			if (_board.ground(index))
			{
				std::stringstream strm;
				strm << TypeEncoder(&oldbible);
				strm << _board.ground(index).type;
				TypeWord typeword = parseTypeWord(strm.str());
				UnitType newtype = newbible.unittype(::stringify(typeword));
				_board.ground(index).type = newtype;
			}
			if (_board.air(index))
			{
				std::stringstream strm;
				strm << TypeEncoder(&oldbible);
				strm << _board.air(index).type;
				TypeWord typeword = parseTypeWord(strm.str());
				UnitType newtype = newbible.unittype(::stringify(typeword));
				_board.air(index).type = newtype;
			}
		}
	}
	catch (const ParseError& error)
	{
		LOGW << "Resetting pooltype due to parse error: " << error.what();
		_pooltype = PoolType::NONE;
		return;
	}

	_bible = std::move(newbible);

	_pooltype = pooltype;

	_owner.closeWorkshop();
	_rulesetname = rulesetname;
	_rulesetbuffer[0] = '\0';
}

void MapEditor::playTestGame()
{
	if (_mapname.empty()) return;

	std::vector<Bot> bots;
	imploding_ptr<Game> game(new LocalGame(
		_gameowner, _owner.settings(),
		getPlayers(_playercount), bots,
		_mapname, _bible.name(),
		_playercount - bots.size(),
		/*silentQuit=*/false, /*record=*/false));
	_gameowner.startGame(std::move(game));
	_cursor->setState(Cursor::State::WAITING);
}

void MapEditor::playVersusAI()
{
	if (_mapname.empty()) return;

	if (_challenge)
	{
		// Save a copy of the map to the expected location for testing.
		saveToFile(AIChallenge::getMapName(Challenge::CUSTOM));
		// Same with the ruleset (even if it is the default ruleset).
		{
			Json::Value json = _bible.toJson();
			std::stringstream strm;
			strm << json;
			std::string fname = Locator::rulesetAuthoredFilename(
				AIChallenge::getRulesetName(Challenge::CUSTOM));
			System::touchFile(fname);
			std::ofstream file = System::ofstream(fname);
			file << strm.rdbuf();
		}
		// Start the challenge.
		_gameowner.startChallenge(Challenge::CUSTOM);
		_cursor->setState(Cursor::State::WAITING);
		return;
	}

	std::string ainame = "dummy";
	if (!AI::pool().empty())
	{
		ainame = AI::pool()[0];
	}
	std::string slotname = "%""A";
	std::vector<Bot> bots;
	for (int i = 0; i < _playercount - 1; i++)
	{
		bots.emplace_back(slotname, ainame, Difficulty::MEDIUM);
		slotname[1] += 1;
	}
	imploding_ptr<Game> game(new LocalGame(
		_gameowner, _owner.settings(),
		getPlayers(_playercount), bots, _mapname, _bible.name(),
		_playercount - bots.size(),
		/*silentQuit=*/false, /*record=*/false));
	_gameowner.startGame(std::move(game));
	_cursor->setState(Cursor::State::WAITING);
}

void MapEditor::switchToRulesetEditor()
{
	_owner.closeWorkshop();
	_showRulesetEditor = true;
	_rulesetname = _bible.name();
	_rulesetbuffer[0] = '\0';
}

void MapEditor::switchToPaletteEditor()
{
	_owner.closeWorkshop();
	_mapname = "colorsample";
	_showRulesetEditor = false;
	_rulesetname = "";
	loadFromFile("colorsample");
	_owner.openPaletteEditor();
}

void MapEditor::quit()
{
	_unsavedCached = unsaved();
	_activepopup = Popup::QUIT;
}

void MapEditor::onConfirmQuit()
{
	_owner.onConfirmQuit();
}

static const char* title(const Popup& popup)
{
	switch (popup)
	{
		case Popup::QUIT: return "Quit?";
		case Popup::NEW: return "New";
		case Popup::OPEN_FILE: return "Open";
		case Popup::COPY_FILE: return "Copy";
		case Popup::SAVE_AS: return "Save As";
		case Popup::SAVE_COPY_AS: return "Save Copy As";
		case Popup::CHANGE_RULESET: return "Change Ruleset";
		case Popup::SAVE_RULESET: return "Save Ruleset";
		case Popup::ERROR: return "Error";

		case Popup::NONE: return nullptr;
	}
	return nullptr;
}

static int filterValidUserContentChar(ImGuiInputTextCallbackData* data)
{
	bool discarded = !isValidUserContentChar(data->EventChar);
	return discarded;
}

void MapEditor::updatePopup()
{
	static std::array<char, 30> inputtext;
	static PoolType inputpooltype;

	if (_activepopup != Popup::NONE && _openedpopup == Popup::NONE)
	{
		inputtext[0] = '\0';

		switch (_activepopup)
		{
			case Popup::CHANGE_RULESET:
			{
				strncpy(inputtext.data(), _bible.name().c_str(),
					inputtext.size() - 1);
				inputpooltype = _pooltype;
			}
			break;

			default:
			break;
		}

		ImGui::OpenPopup(title(_activepopup));
		_openedpopup = _activepopup;
	}

	if (_openedpopup == Popup::NONE) return;

	if (!ImGui::BeginPopupModal(title(_openedpopup), NULL,
			ImGuiWindowFlags_AlwaysAutoResize))
	{
		return;
	}

	if (Input::get()->wasKeyPressed(SDL_SCANCODE_ESCAPE))
	{
		if (_openedpopup == Popup::QUIT)
		{
			if (!_unsavedCached)
			{
				_activepopup = Popup::NONE;
				onConfirmQuit();
			}
			// else {}
		}
		else _activepopup = Popup::NONE;
	}

	switch (_openedpopup)
	{
		case Popup::QUIT:
		{
			if (_unsavedCached)
			{
				ImGui::Text("There are unsaved changes.");
			}
			if (ImGui::Button("Quit"))
			{
				_activepopup = Popup::NONE;
				onConfirmQuit();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) _activepopup = Popup::NONE;
		}
		break;

		case Popup::NEW:
		{
			if (_unsavedCached)
			{
				ImGui::Text("There are unsaved changes.");
				ImGui::Separator();
			}
			ImGui::DragInt("Columns", &_cols, 1, 1, Position::MAX_COLS);
			ImGui::DragInt("Rows", &_rows, 1, 1, Position::MAX_ROWS);
			if (ImGui::Button("New"))
			{
				_activepopup = Popup::NONE;
				onNewDimensions(_cols, _rows);
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) _activepopup = Popup::NONE;
		}
		break;

		case Popup::OPEN_FILE:
		{
			if (_unsavedCached)
			{
				ImGui::Text("There are unsaved changes.");
				ImGui::Separator();
			}
			if (!_cachedMapNames.empty())
			{
				if (ImGui::BeginChild("cached map names", ImVec2(0, 100)))
				{
					for (const std::string& name : _cachedMapNames)
					{
						if (ImGui::Selectable(name.c_str()))
						{
							if (name.size() < inputtext.size() - 1)
							{
								inputtext.fill('\0');
								std::copy(name.begin(), name.end(),
									inputtext.data());
							}
						}
					}
				}
				ImGui::EndChild();
			}
			ImGui::InputText("Map name", inputtext.data(), inputtext.size(),
				ImGuiInputTextFlags_CallbackCharFilter,
				filterValidUserContentChar);
			bool exists = false;
			if (strlen(inputtext.data()) >= 3)
			{
				if (Map::exists(inputtext.data()))
				{
					exists = true;
				}
				else
				{
					static ImVec4 invalidcolor(1.0f, 0.4f, 0.2f, 1.0f);
					ImGui::TextColored(invalidcolor, "Not found");
				}
			}
			if (exists)
			{
				if (ImGui::Button("Open"))
				{
					_activepopup = Popup::NONE;
					onOpenMapName(inputtext.data());
				}
				ImGui::SameLine();
			}
			if (ImGui::Button("Cancel")) _activepopup = Popup::NONE;
		}
		break;

		case Popup::COPY_FILE:
		{
			if (_unsavedCached)
			{
				ImGui::Text("There are unsaved changes.");
				ImGui::Separator();
			}
			if (!_cachedMapNames.empty())
			{
				if (ImGui::BeginChild("cached map names", ImVec2(0, 100)))
				{
					for (const std::string& name : _cachedMapNames)
					{
						if (ImGui::Selectable(name.c_str()))
						{
							if (name.size() < inputtext.size() - 1)
							{
								inputtext.fill('\0');
								std::copy(name.begin(), name.end(),
									inputtext.data());
							}
						}
					}
				}
				ImGui::EndChild();
			}
			ImGui::InputText("Map name", inputtext.data(), inputtext.size(),
				ImGuiInputTextFlags_CallbackCharFilter,
				filterValidUserContentChar);
			bool exists = false;
			if (strlen(inputtext.data()) >= 3)
			{
				if (Map::exists(inputtext.data()))
				{
					exists = true;
				}
				else
				{
					static ImVec4 invalidcolor(1.0f, 0.4f, 0.2f, 1.0f);
					ImGui::TextColored(invalidcolor, "Not found");
				}
			}
			if (exists)
			{
				if (ImGui::Button("Copy"))
				{
					_activepopup = Popup::NONE;
					onCopyMapName(inputtext.data());
				}
				ImGui::SameLine();
			}
			if (ImGui::Button("Cancel")) _activepopup = Popup::NONE;
		}
		break;

		case Popup::SAVE_AS:
		{
			ImGui::InputText("Map name", inputtext.data(), inputtext.size(),
				ImGuiInputTextFlags_CallbackCharFilter,
				filterValidUserContentChar);
			if (isValidUserContentName(inputtext.data()))
			{
				if (ImGui::Button("Save"))
				{
					_activepopup = Popup::NONE;
					onSaveAsMapName(inputtext.data());
				}
				ImGui::SameLine();
			}
			if (ImGui::Button("Cancel")) _activepopup = Popup::NONE;
		}
		break;

		case Popup::SAVE_COPY_AS:
		{
			ImGui::InputText("Map name", inputtext.data(), inputtext.size(),
				ImGuiInputTextFlags_CallbackCharFilter,
				filterValidUserContentChar);
			if (isValidUserContentName(inputtext.data()))
			{
				if (ImGui::Button("Save"))
				{
					_activepopup = Popup::NONE;
					onSaveCopyAsMapName(inputtext.data());
				}
				ImGui::SameLine();
			}
			if (ImGui::Button("Cancel")) _activepopup = Popup::NONE;
		}
		break;

		case Popup::CHANGE_RULESET:
		{
			if (_unsavedCached)
			{
				ImGui::Text("There are unsaved changes.");
				ImGui::Separator();
			}

			int tp = static_cast<int>(inputpooltype);
			int oldtp = tp;
			for (int i = 0; i < (int) POOLTYPE_SIZE; i++)
			{
				PoolType pool = static_cast<PoolType>(i);
				const char* txt = ::stringify(pool);
				if (i > 0) ImGui::SameLine();
				ImGui::RadioButton(txt, &tp, i);
			}
			if (tp != oldtp)
			{
				inputpooltype = static_cast<PoolType>(tp);
				switch (inputpooltype)
				{
					case PoolType::MULTIPLAYER:
					{
						std::string rulesetname = Library::nameCurrentBible();
						strncpy(inputtext.data(), rulesetname.c_str(),
							inputtext.size() - 1);
					}
					break;

					default:
					break;
				}
			}

			if (inputpooltype != PoolType::MULTIPLAYER)
			{
				if (!_cachedRulesetNames.empty())
				{
					if (ImGui::BeginChild("cached names", ImVec2(0, 100)))
					{
						for (const std::string& name : _cachedRulesetNames)
						{
							if (ImGui::Selectable(name.c_str()))
							{
								if (name.size() < inputtext.size() - 1)
								{
									inputtext.fill('\0');
									std::copy(name.begin(), name.end(),
										inputtext.data());
								}
							}
						}
					}
					ImGui::EndChild();
				}
				ImGui::InputText("Ruleset", inputtext.data(), inputtext.size(),
					ImGuiInputTextFlags_CallbackCharFilter,
					filterValidUserContentChar);
			}
			else
			{
				ImGui::LabelText("Ruleset", " %s", inputtext.data());
			}
			bool exists = false;
			if (strlen(inputtext.data()) >= 3)
			{
				if (Library::existsBible(inputtext.data()))
				{
					exists = true;
				}
				else
				{
					static ImVec4 invalidcolor(1.0f, 0.4f, 0.2f, 1.0f);
					ImGui::TextColored(invalidcolor, "Not found");
				}
			}
			if (exists)
			{
				if (ImGui::Button("Change"))
				{
					_activepopup = Popup::NONE;
					onChangeRuleset(inputpooltype, inputtext.data());
				}
				ImGui::SameLine();
			}
			if (ImGui::Button("Cancel")) _activepopup = Popup::NONE;
		}
		break;

		case Popup::SAVE_RULESET:
		{
			ImGui::InputText("Ruleset", inputtext.data(), inputtext.size(),
				ImGuiInputTextFlags_CallbackCharFilter,
				filterValidUserContentChar);
			bool valid = true;
			if (strlen(inputtext.data()) >= 3)
			{
				if (inputtext[0] == 'v'
					&& inputtext[1] >= '0' && inputtext[1] <= '9')
				{
					static ImVec4 invalidcolor(1.0f, 0.4f, 0.2f, 1.0f);
					ImGui::TextColored(invalidcolor, "Invalid name");
					valid = false;
				}
				else if (Library::existsBible(inputtext.data()))
				{
					static ImVec4 invalidcolor(1.0f, 0.4f, 0.2f, 1.0f);
					ImGui::TextColored(invalidcolor, "Overwrite?");
				}
			}
			if (valid)
			{
				if (ImGui::Button("Save"))
				{
					_owner.closeWorkshop();
					_rulesetname = inputtext.data();
					_rulesetsaving = true;
					_activepopup = Popup::NONE;
				}
				ImGui::SameLine();
			}
			if (ImGui::Button("Cancel"))
			{
				_activepopup = Popup::NONE;
			}
		}
		break;

		case Popup::ERROR:
		{
			ImGui::Text("An unexpected error occurred.");
			if (ImGui::Button("Ok")) _activepopup = Popup::NONE;
		}
		break;

		case Popup::NONE:
		break;
	}

	if (_activepopup != _openedpopup)
	{
		ImGui::CloseCurrentPopup();
		_openedpopup = Popup::NONE;
	}

	ImGui::EndPopup();
}

void ChallengeEditData::load(const Json::Value& root)
{
	if (!root["challenge"].isObject()) return;
	const Json::Value& challenge = root["challenge"];

	if (challenge["name"].isString())
	{
		strncpy(name.data(), challenge["name"].asString().c_str(),
			name.size() - 1);
	}
	if (challenge["max_stars"].isInt())
	{
		stars = challenge["max_stars"].asInt();
		if (stars > 1)
		{
			advanced = true;
		}
	}

	if (!challenge["briefing"].isObject()) return;
	const Json::Value& briefing = challenge["briefing"];

	std::tuple<AIChallenge::Brief, char*, size_t> tuples[] = {
		std::tuple<AIChallenge::Brief, char*, size_t>{
			AIChallenge::Brief::GREETING, greeting.data(), greeting.size()},
		std::tuple<AIChallenge::Brief, char*, size_t>{
			AIChallenge::Brief::DESCRIPTION,
			description.data(), description.size()},
		std::tuple<AIChallenge::Brief, char*, size_t>{
			AIChallenge::Brief::OBJECTIVE,
			objective.data(), objective.size()},
		std::tuple<AIChallenge::Brief, char*, size_t>{
			AIChallenge::Brief::FIRST_STAR, one.data(), one.size()},
		std::tuple<AIChallenge::Brief, char*, size_t>{
			AIChallenge::Brief::SECOND_STAR, two.data(), two.size()},
		std::tuple<AIChallenge::Brief, char*, size_t>{
			AIChallenge::Brief::THIRD_STAR, three.data(), three.size()},
		std::tuple<AIChallenge::Brief, char*, size_t>{
			AIChallenge::Brief::SENDOFF, sendoff.data(), sendoff.size()},
	};
	for (auto tuple : tuples)
	{
		AIChallenge::Brief brief;
		char* data;
		size_t size;
		std::tie(brief, data, size) = tuple;
		const Json::Value& value = briefing[AIChallenge::stringify(brief)];
		if (value.isString())
		{
			strncpy(data, value.asString().c_str(), size - 1);
		}
		switch (brief)
		{
			case AIChallenge::Brief::FIRST_STAR:
			case AIChallenge::Brief::SECOND_STAR:
			case AIChallenge::Brief::THIRD_STAR:
			{
				if (value.isString() && !value.asString().empty())
				{
					advanced = true;
				}
			}
			break;
			default:
			break;
		}
	}
}

void ChallengeEditData::store(Json::Value& root)
{
	Json::Value challenge = Json::objectValue;
	challenge["name"] = name.data();
	challenge["max_stars"] = stars;
	Json::Value briefing = Json::objectValue;
	std::tuple<AIChallenge::Brief, const char*> tuples[] = {
		std::tuple<AIChallenge::Brief, const char*>{
			AIChallenge::Brief::GREETING, greeting.data()},
		std::tuple<AIChallenge::Brief, const char*>{
			AIChallenge::Brief::DESCRIPTION, description.data()},
		std::tuple<AIChallenge::Brief, const char*>{
			AIChallenge::Brief::OBJECTIVE, objective.data()},
		std::tuple<AIChallenge::Brief, const char*>{
			AIChallenge::Brief::FIRST_STAR, one.data()},
		std::tuple<AIChallenge::Brief, const char*>{
			AIChallenge::Brief::SECOND_STAR, two.data()},
		std::tuple<AIChallenge::Brief, const char*>{
			AIChallenge::Brief::THIRD_STAR, three.data()},
		std::tuple<AIChallenge::Brief, const char*>{
			AIChallenge::Brief::SENDOFF, sendoff.data()},
	};
	for (auto tuple : tuples)
	{
		AIChallenge::Brief brief;
		const char* data;
		std::tie(brief, data) = tuple;
		if (brief == AIChallenge::Brief::SECOND_STAR && stars < 2) continue;
		if (brief == AIChallenge::Brief::THIRD_STAR && stars < 3) continue;
		briefing[AIChallenge::stringify(brief)] = data;
	}
	challenge["briefing"] = std::move(briefing);
	root["challenge"] = std::move(challenge);

}
