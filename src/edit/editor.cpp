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
#include "level.hpp"
#include "surface.hpp"
#include "cursor.hpp"
#include "changeset.hpp"
#include "automaton.hpp"
#include "map.hpp"
#include "spritepattern.hpp"
#include "recording.hpp"
#include "typenamer.hpp"
#include "parseerror.hpp"
#include "treetype.hpp"
#include "palette.hpp"
#include "colorname.hpp"
#include "skin.hpp"
#include "language.hpp"
#include "system.hpp"


static std::vector<Player> allPlayers;

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
};

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

Editor::Editor(Settings& settings, const std::string& filename) :
	_settings(settings),
	_displaysettings(_settings),
	_loop(*this, _settings.framerate.value()),
	_graphics(_settings),
	_renderer(_graphics),
	_camera(_settings, _graphics.width(), _graphics.height()),
	_camerafocus(nullptr),
	_mixer(_settings),
	_filename(filename),
	_pooltype(PoolType::NONE),
	_savedpooltype(PoolType::NONE),
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

	// Load the most recent bible. This works because the Level and the Boards
	// have a reference to _bible, which remains valid when we change its data.
	_bible = _library.get(_library.currentRuleset());

	_camerafocus.reset(new CameraFocus(_settings, Surface::WIDTH));
	_camerafocus->set(_level.centerPoint());
	_camerafocus->load(_level.topleftPoint(), _level.bottomrightPoint());
	_camerafocus->stopActing();

	_cursor.reset(new Cursor(_board, _bible));
	_cursor->setState(Cursor::State::ACTIVE);

	if (_settings.seed.defined())
	{
		srand(_settings.seed.value());
	}
	else
	{
		auto timestampMs = SteadyClock::milliseconds();
		srand(timestampMs);
	}

	if (!_filename.empty())
	{
		load();
	}
	else
	{
		loadEmpty();
	}
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
	_unsavedCached = unsaved();
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

	if (_activepopup == Popup::NONE
		&& _input.wasKeyPressed(SDL_SCANCODE_ESCAPE))
	{
		quit();
	}

	ImGuiSDL::NewFrame(_graphics.getWindow());
	ImGui::NewFrame();
	_graphics.clear();
	/* START OF UPDATES */

	_camerafocus->update();
	_camera.update();
	_mixer.update();

	if (_cursor)
	{
		Input* input = Input::get();
		Point point(input->mousePoint());
		Position position(Surface::convert(point));
		Cell index = _level.index(position);
		_cursor->set(index);
	}

	updatePaintMode();
	updatePaintBrush();
	updateParameters();
	updateGlobals();
	_skineditor.updatePalettes();
	_skineditor.updateSkins();
	updateMenuBar();
	updatePopup();

	_cursor->update();

	_level.update();

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

void Editor::updateMenuBar()
{
	if (_input.isKeyHeld(SDL_SCANCODE_CTRL))
	{
		if (_input.wasKeyPressed(SDL_SCANCODE_S))
		{
			if (_input.isKeyHeld(SDL_SCANCODE_ALT))
			{
				saveCopyAs();
			}
			else if (_filename.empty() || _input.isKeyHeld(SDL_SCANCODE_SHIFT))
			{
				saveAs();
			}
			else
			{
				save();
			}
		}
		else if (_input.wasKeyPressed(SDL_SCANCODE_O))
		{
			if (_input.isKeyHeld(SDL_SCANCODE_ALT))
			{
				loadCopyFrom();
			}
			else
			{
				loadFrom();
			}
		}
		else if (_input.wasKeyPressed(SDL_SCANCODE_N))
		{
			clear();
		}
	}

	if (ImGui::BeginMainMenuBar())
	{
		const char *fileheader = _unsavedCached ? "File*" : "File ";
		if (ImGui::BeginMenu(fileheader))
		{
			std::stringstream dispname;
			if (_filename.empty())
				dispname << "(new file)";
			else
				dispname << "(file: " << _filename << ")";
			ImGui::MenuItem(dispname.str().c_str(), NULL, false, false);
			ImGui::Separator();
			if (ImGui::MenuItem("New File...", "Ctrl+N"))
			{
				clear();
			}
			if (ImGui::MenuItem("Open File...", "Ctrl+O"))
			{
				loadFrom();
			}
			if (ImGui::MenuItem("Copy File...", "Ctrl+Alt+O"))
			{
				loadCopyFrom();
			}
			if (ImGui::MenuItem("Save", "Ctrl+S"))
			{
				if (_filename.empty())
					saveAs();
				else
					save();
			}
			if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
			{
				saveAs();
			}
			if (ImGui::MenuItem("Save Copy As...", "Ctrl+Alt+S"))
			{
				saveCopyAs();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Quit...", "Esc, Esc, Esc"))
			{
				quit();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

void Editor::updatePaintMode()
{
	{
		_paintdesc.position = Surface::convert(_input.mousePoint());
	}

	if (ImGui::Begin("Paint Brush", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		int md = static_cast<int>(_paintdesc.type);
		ImGui::RadioButton("Tile",   &md, static_cast<int>(Descriptor::Type::TILE));
		ImGui::RadioButton("Ground", &md, static_cast<int>(Descriptor::Type::GROUND));
		ImGui::RadioButton("Air"  ,  &md, static_cast<int>(Descriptor::Type::AIR));
		ImGui::RadioButton("Hazards",&md, static_cast<int>(Descriptor::Type::CELL));
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
		ImGui::Text("Temp.: %i", square->temperature());
		ImGui::Text("Hum.: %i", square->humidity());
	}
	ImGui::End();
}

void Editor::updatePaintBrush()
{
	Cell index = _board.cell(_paintdesc.position);
	if (_input.isKeyHeld(SDL_SCANCODE_LMB) && !index.edge())
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
	else if (_input.isKeyHeld(SDL_SCANCODE_RMB) && !index.edge())
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

void Editor::updateParameters()
{
	if (ImGui::Begin("Parameters", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Map Pool: %s", ::stringify(_pooltype));
		ImGui::Text("Ruleset: %s", _bible.name().c_str());
		if (ImGui::Button("Change")) changeRuleset();
		ImGui::Separator();

		ImGui::SliderInt("Players", &_playercount, 2, PLAYER_MAX);
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

void Editor::updateGlobals()
{
	int totaltemperature = 0;
	int totalhumidity = 0;
	int totalaccessible = 0;
	int totalwalkable = 0;
	int totalgrassy = 0;
	int count = (int) _rows * _cols;
	for (Cell index : _board)
	{
		totaltemperature += _board.temperature(index);
		totalhumidity += _board.humidity(index);
		if (_bible.tileAccessible(_board.tile(index).type)) totalaccessible++;
		if (_bible.tileWalkable(_board.tile(index).type)) totalwalkable++;
		if (_bible.tileGrassy(_board.tile(index).type)) totalgrassy++;
	}

	int averagetemperature = (totaltemperature + count / 2) / count;
	int averagehumidity = (totalhumidity + count / 2) / count;

	if (ImGui::Begin("Info", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::PushItemWidth(80);

		int t = averagetemperature;
		ImGui::InputInt("Avg. temp.", &t, 5, -25, 25);
		if (t != averagetemperature)
		{
			for (int r = 0; r < _rows; r++)
			{
				for (int c = 0; c < _cols; c++)
				{
					Change change(Change::Type::TEMPERATURE, Descriptor::cell(Position(r, c)));
					change.xTemperature(t - averagetemperature);
					_board.enact(change);
					_level.enact(change, nullptr);
				}
			}
		}

		int h = averagehumidity;
		ImGui::InputInt("Avg. hum.", &h, 1, 0, 4);
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

		if ((t != averagetemperature || h != averagehumidity)
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

		ImGui::Separator();
		ImGui::Text("Rows: %d", _board.rows());
		if (ImGui::Button("Expand Top")) expandTop();
		if (ImGui::Button("Expand Bottom")) expandBottom();
		if (ImGui::Button("Crop Top")) cropTop();
		if (ImGui::Button("Crop Bottom")) cropBottom();
		ImGui::Text("Columns: %d", _board.cols());
		if (ImGui::Button("Expand Left")) expandLeft();
		if (ImGui::Button("Expand Right")) expandRight();
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

void Editor::generateBiomes()
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

void Editor::clear()
{
	_unsavedCached = unsaved();
	_activepopup = Popup::NEW;
}

void Editor::onNewDimensions(int cols, int rows)
{
	_cols = cols;
	_rows = rows;
	_board.clear(_cols, _rows);
	_level.clear(_cols, _rows);

	loadEmpty();

	_filename = "";
}

void Editor::expandTop()
{
	ChangeSet changes;
	store(Position(/*row=*/-1, /*col=*/0), changes);

	_rows += 1;
	_board.clear(_cols, _rows);
	_level.clear(_cols, _rows);

	restore(changes);
}

void Editor::expandLeft()
{
	ChangeSet changes;
	store(Position(/*row=*/0, /*col=*/-1), changes);

	_cols += 1;
	_board.clear(_cols, _rows);
	_level.clear(_cols, _rows);

	restore(changes);
}

void Editor::expandRight()
{
	ChangeSet changes;
	store(Position(/*row=*/0, /*col=*/0), changes);

	_cols += 1;
	_board.clear(_cols, _rows);
	_level.clear(_cols, _rows);

	restore(changes);
}

void Editor::expandBottom()
{
	ChangeSet changes;
	store(Position(/*row=*/0, /*col=*/0), changes);

	_rows += 1;
	_board.clear(_cols, _rows);
	_level.clear(_cols, _rows);

	restore(changes);
}

void Editor::cropTop()
{
	ChangeSet changes;
	store(Position(/*row=*/+1, /*col=*/0), changes);

	_rows -= 1;
	_board.clear(_cols, _rows);
	_level.clear(_cols, _rows);

	restore(changes);
}

void Editor::cropLeft()
{
	ChangeSet changes;
	store(Position(/*row=*/0, /*col=*/+1), changes);

	_cols -= 1;
	_board.clear(_cols, _rows);
	_level.clear(_cols, _rows);

	restore(changes);
}

void Editor::cropRight()
{
	ChangeSet changes;
	store(Position(/*row=*/0, /*col=*/0), changes);

	_cols -= 1;
	_board.clear(_cols, _rows);
	_level.clear(_cols, _rows);

	restore(changes);
}

void Editor::cropBottom()
{
	ChangeSet changes;
	store(Position(/*row=*/0, /*col=*/0), changes);

	_rows -= 1;
	_board.clear(_cols, _rows);
	_level.clear(_cols, _rows);

	restore(changes);
}

void Editor::store(const Position& topleft, ChangeSet& changes)
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

void Editor::restore(const ChangeSet& changes)
{
	for (const Change& change : changes.get(Player::OBSERVER))
	{
		if (change.subject.type != Descriptor::Type::NONE
			&& (change.subject.position.row >= _board.rows()
				|| change.subject.position.col >= _board.cols())) continue;

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

void Editor::loadEmpty()
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

	saved();

	loaded();
}

void Editor::load()
{
	loadFromFile(_filename);
}

void Editor::loadFrom()
{
	_activepopup = Popup::OPEN_FILE;
}

void Editor::loadCopyFrom()
{
	_activepopup = Popup::COPY_FILE;
}

void Editor::onOpenFileFilename(const std::string &fname)
{
	_filename = fname;
	loadFromFile(fname);
}

void Editor::onCopyFileFilename(const std::string &fname)
{
	loadFromFile(fname);
}

void Editor::loadFromFile(const std::string& mapname)
{
	// TODO fname is not really the filename (#23)

	Json::Reader reader;
	Json::Value metadata;
	std::string line;

	std::string fname = Map::readOnlyFilename(mapname);
	std::ifstream file = System::ifstream(fname);
	if (!std::getline(file, line) || !reader.parse(line, metadata)
		|| !metadata.isObject())
	{
		// Try old style.
		file.close();
		file.open(fname);
		if (!reader.parse(file, metadata) || !metadata.isObject())
		{
			LOGF << "Failed to load map '" << mapname << "' (" << fname << "): "
				<< reader.getFormattedErrorMessages();
			throw std::runtime_error("failed to load map " + mapname + ": \t"
					+ reader.getFormattedErrorMessages());
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
		// TODO replace with friendly error message in the UI
		throw std::runtime_error("version mismatch while parsing map");
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
			rulesetname = _library.currentRuleset();
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
			rulesetname = _library.currentRuleset();
		}
		else rulesetname = _library.compatibleRuleset(bibleversion);
	}
	else
	{
		rulesetname = _library.compatibleRuleset(Version::prehistoric());
	}

	// Look for a bible with that version.
	if (!_library.exists(rulesetname))
	{
		// Backwards compatibility: retry with a compatible ruleset.
		rulesetname = _library.compatibleRuleset(rulesetname);

		if (!_library.exists(rulesetname))
		{
			// TODO replace with friendly error message in the UI
			throw std::runtime_error("missing bible '" + rulesetname + "'"
				" while parsing map");
		}
	}

	// Get the bible version.
	Version bibleversion = _library.get(rulesetname).version();

	// Check compatibility.
	if ((bibleversion.major > myversion.major
			|| (bibleversion.minor == myversion.minor
				&& bibleversion.minor > myversion.minor))
		&& !bibleversion.isDevelopment()
		&& !myversion.isDevelopment())
	{
		// TODO replace with friendly error message in the UI
		throw std::runtime_error("version mismatch while parsing map");
	}

	// Load the bible. This works because the Level and the Boards have a
	// reference to _bible, which remains valid when we change its data.
	if (_pooltype == PoolType::MULTIPLAYER)
	{
		// For multiplayer maps, always use the most recent ruleset.
		_bible = _library.get(_library.currentRuleset());
	}
	else
	{
		// For custom maps, load the named ruleset.
		_bible = _library.get(rulesetname);
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
			_bible = _library.get(rulesetname);

			// Try again. If this also fails, there is actually something wrong.
			_board.load(mapname);
		}
		else throw;
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

	saved();

	loaded();

	if (!validpooltype)
	{
		// If loading the board failed because the map features tiles or units
		// that no longer exist in the newest ruleset, then the user must
		// manually set the pooltype again to resolve this.
		_pooltype = PoolType::NONE;
	}
}

void Editor::loaded()
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

void Editor::save()
{
	saveToFile(_filename);
}

void Editor::saveAs()
{
	_activepopup = Popup::SAVE_AS;
}

void Editor::saveCopyAs()
{
	_activepopup = Popup::SAVE_COPY_AS;
}

void Editor::onSaveAsFilename(const std::string &fname)
{
	_filename = fname;
	saveToFile(fname);
}

void Editor::onSaveCopyAsFilename(const std::string &fname)
{
	saveToFile(fname);
}

void Editor::saveToFile(const std::string &fname)
{
	// TODO fname is not really the filename (#23)
	const std::string& mapname = fname;

	std::ofstream file = System::ofstream(Map::authoredFilename(mapname));

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

void Editor::saved()
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

bool Editor::unsaved()
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

void Editor::changeRuleset()
{
	_activepopup = Popup::CHANGE_RULESET;
}

void Editor::onChangeRuleset(const PoolType& pooltype,
		const std::string& rulesetname)
{
	if (!_library.exists(rulesetname))
	{
		// TODO replace with friendly error message in the UI
		throw std::runtime_error("Unknown ruleset '" + rulesetname + "'");
		return;
	}

	Bible& oldbible = _bible;
	Bible newbible = _library.get(rulesetname);

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
}

void Editor::quit()
{
	_unsavedCached = unsaved();
	_activepopup = Popup::QUIT;
}

void Editor::onConfirmQuit()
{
	_loop.stop();
}

static const char* title(const Popup& popup)
{
	switch (popup)
	{
		case Popup::QUIT: return "Quit?";
		case Popup::NEW: return "New";
		case Popup::OPEN_FILE: return "Open File";
		case Popup::COPY_FILE: return "Copy File";
		case Popup::SAVE_AS: return "Save As";
		case Popup::SAVE_COPY_AS: return "Save Copy As";
		case Popup::CHANGE_RULESET: return "Change Ruleset";

		case Popup::NONE: return nullptr;
	}
	return nullptr;
}

void Editor::updatePopup()
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

	if (_input.wasKeyPressed(SDL_SCANCODE_ESCAPE))
	{
		if (_openedpopup == Popup::QUIT)
		{
			if (!_unsavedCached)
			{
				onConfirmQuit();
				_activepopup = Popup::NONE;
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
				onConfirmQuit();
				_activepopup = Popup::NONE;
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
				onNewDimensions(_cols, _rows);
				_activepopup = Popup::NONE;
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
			ImGui::InputText("Filename", inputtext.data(), inputtext.size());
			if (ImGui::Button("Open"))
			{
				onOpenFileFilename(inputtext.data());
				_activepopup = Popup::NONE;
			}
			ImGui::SameLine();
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
			ImGui::InputText("Filename", inputtext.data(), inputtext.size());
			if (ImGui::Button("Copy"))
			{
				onCopyFileFilename(inputtext.data());
				_activepopup = Popup::NONE;
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) _activepopup = Popup::NONE;
		}
		break;

		case Popup::SAVE_AS:
		{
			ImGui::InputText("Filename", inputtext.data(), inputtext.size());
			if (ImGui::Button("Save"))
			{
				onSaveAsFilename(inputtext.data());
				_activepopup = Popup::NONE;
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) _activepopup = Popup::NONE;
		}
		break;

		case Popup::SAVE_COPY_AS:
		{
			ImGui::InputText("Filename", inputtext.data(), inputtext.size());
			if (ImGui::Button("Save"))
			{
				onSaveCopyAsFilename(inputtext.data());
				_activepopup = Popup::NONE;
			}
			ImGui::SameLine();
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
						std::string rulesetname = _library.currentRuleset();
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
				ImGui::InputText("Ruleset", inputtext.data(), inputtext.size());
			}
			else
			{
				ImGui::LabelText("Ruleset", " %s", inputtext.data());
			}
			if (ImGui::Button("Change"))
			{
				onChangeRuleset(inputpooltype, inputtext.data());
				_activepopup = Popup::NONE;
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) _activepopup = Popup::NONE;
		}
		break;

		case Popup::NONE:
		break;
	}

	if (_activepopup == Popup::NONE)
	{
		ImGui::CloseCurrentPopup();
		_openedpopup = Popup::NONE;
	}

	ImGui::EndPopup();
}
