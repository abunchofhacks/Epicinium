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
#include "board.hpp"
#include "source.hpp"

#include "change.hpp"
#include "player.hpp"
#include "map.hpp"
#include "typenamer.hpp"
#include "system.hpp"


Board::Board(const TypeNamer& typenamer) :
	_typenamer(typenamer)
{
	resize(20, 13);
}

void Board::clear(int cols, int rows)
{
	resize(cols, rows);
}

void Board::resize(int cols, int rows)
{
	DEBUG_ASSERT(cols <= Position::MAX_COLS && rows <= Position::MAX_ROWS);
	if (cols > Position::MAX_COLS)
	{
		cols = Position::MAX_COLS;
	}
	if (rows > Position::MAX_ROWS)
	{
		rows = Position::MAX_ROWS;
	}

	_spaces.clear();

	_cols = cols;
	_rows = rows;

	int length = _rows * _cols + 1;
	_spaces.reserve(length);
	for (int r = 0; r < _rows; r++)
	{
		for (int c = 0; c < _cols; c++)
		{
			_spaces.emplace_back(r, c);
		}
	}
	_spaces.emplace_back(-1, -1);
}

void Board::load(const std::string& mapname)
{
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

	int cols = metadata["cols"].asInt();
	int rows = metadata["rows"].asInt();
	if (cols != _cols || _rows != rows)
	{
		resize(cols, rows);
	}

	// Old style: cells are included in the json.
	if (metadata["cells"].isArray())
	{
		Json::ValueConstIterator iter = metadata["cells"].begin();
		for (Cell index : cells())
		{
			const Json::Value& celljson = *iter;
			loadCellFromJson(index, celljson);
			iter++;
		}
	}
	// New style: each line is its own celljson.
	else
	{
		for (Cell index : cells())
		{
			Json::Value celljson;
			if (!std::getline(file, line) || !reader.parse(line, celljson))
			{
				throw std::runtime_error("error while parsing cells");
			}
			loadCellFromJson(index, celljson);
		}
	}
}

void Board::loadCellFromJson(Cell index, const Json::Value& celljson)
{
	{
		tile(index) = TileToken(_typenamer, celljson["tile"]);
	}
	{
		ground(index) = UnitToken(_typenamer, celljson["ground"]);
	}
	{
		air(index) = UnitToken(_typenamer, celljson["air"]);
	}

	if (celljson["temperature"].isInt())
	{
		temperature(index) = celljson["temperature"].asInt();
	}
	if (celljson["humidity"].isInt())
	{
		humidity(index) = celljson["humidity"].asInt();
	}
	if (celljson["chaos"].isInt())
	{
		chaos(index) = celljson["chaos"].asInt();
	}
	if (celljson["gas"].isInt())
	{
		gas(index) = celljson["gas"].asInt();
	}
	if (celljson["radiation"].isInt())
	{
		radiation(index) = celljson["radiation"].asInt();
	}

	if (celljson["snow"].isBool())
	{
		snow(index) = celljson["snow"].asBool();
	}
	if (celljson["frostbite"].isBool())
	{
		frostbite(index) = celljson["frostbite"].asBool();
	}
	if (celljson["firestorm"].isBool())
	{
		firestorm(index) = celljson["firestorm"].asBool();
	}
	if (celljson["bonedrought"].isBool())
	{
		bonedrought(index) = celljson["bonedrought"].asBool();
	}
	if (celljson["death"].isBool())
	{
		death(index) = celljson["death"].asBool();
	}
}

// Note the pass by value, since we will locally shuffle the players in this function.
void Board::assignPlayers(std::vector<Player> players)
{
	_players = players;

	std::map<Player, Player> assignments;

	for (Cell index : cells())
	{
		if (tile(index).owner != Player::NONE)
		{
			assignments[tile(index).owner] = Player::NONE;
		}
		if (ground(index).owner != Player::NONE)
		{
			assignments[ground(index).owner] = Player::NONE;
		}
		if (air(index).owner != Player::NONE)
		{
			assignments[air(index).owner] = Player::NONE;
		}
	}

	for (size_t i = players.size(); i < assignments.size(); i++)
	{
		players.push_back(Player::NONE);
	}

	std::random_shuffle(players.begin(), players.end());

	{
		size_t i = 0;
		auto iter = assignments.begin();
		for (; iter != assignments.end(); ++iter, ++i)
		{
			iter->second = players[i];
		}
	}

	for (Cell index : cells())
	{
		if (tile(index).owner != Player::NONE)
		{
			tile(index).owner = assignments[tile(index).owner];
		}

		if (ground(index).owner != Player::NONE)
		{
			if (assignments[ground(index).owner] != Player::NONE)
			{
				ground(index).owner = assignments[ground(index).owner];
			}
			else
			{
				ground(index) = UnitToken();
			}
		}

		if (air(index).owner    != Player::NONE)
		{
			if (assignments[air(index).owner] != Player::NONE)
			{
				air(index).owner = assignments[air(index).owner];
			}
			else
			{
				air(index) = UnitToken();
			}
		}
	}
}

void Board::setPlayers(std::vector<Player> players)
{
	_players = players;
}

std::vector<Player> Board::players()
{
	return _players;
}

void Board::enact(const Change& change)
{
	switch (change.type)
	{
		case Change::Type::STARTS:
		break;

		case Change::Type::MOVES:
		{
			Cell from = cell(change.subject.position);
			Cell to   = cell(change.target.position);
			// Swap the unit data.
			std::swap(unit(from, change.subject.type), unit(to, change.target.type));
		}
		break;

		case Change::Type::REVEAL:
		{
			Cell index = cell(change.subject.position);
			vision(index).add(Player::SELF);
			// Update the tile.
			tile(index) = change.tile;
			tile(index).resetId();
			// Update the markers and counters.
			snow(index) = change.snow;
			frostbite(index) = change.frostbite;
			firestorm(index) = change.firestorm;
			bonedrought(index) = change.bonedrought;
			death(index) = change.death;
			gas(index) = change.gas;
			radiation(index) = change.radiation;
			temperature(index) = change.temperature;
			humidity(index) = change.humidity;
			chaos(index) = change.chaos;
		}
		break;

		case Change::Type::OBSCURE:
		{
			Cell index = cell(change.subject.position);
			vision(index).remove(Player::SELF);
		}
		break;

		case Change::Type::TRANSFORMED:
		{
			Cell index = cell(change.subject.position);
			tile(index) = change.tile;
			tile(index).resetId();
		}
		break;

		case Change::Type::CONSUMED:
		{
			Cell index = cell(change.subject.position);
			tile(index) = change.tile;
			tile(index).resetId();
		}
		break;

		case Change::Type::SHAPES:
		break;

		case Change::Type::SHAPED:
		{
			Cell index = cell(change.subject.position);
			tile(index) = change.tile;
			tile(index).resetId();
		}
		break;

		case Change::Type::SETTLES:
		{
			Cell index = cell(change.subject.position);
			unit(index, change.subject.type) = UnitToken();
		}
		break;

		case Change::Type::SETTLED:
		{
			Cell index = cell(change.subject.position);
			tile(index) = change.tile;
			tile(index).resetId();
		}
		break;

		case Change::Type::EXPANDS:
		{
			Cell index = cell(change.subject.position);
			tile(index).power += change.power;
		}
		break;

		case Change::Type::EXPANDED:
		{
			Cell index = cell(change.subject.position);
			tile(index) = change.tile;
			tile(index).resetId();
		}
		break;

		case Change::Type::UPGRADES:
		{
			Cell index = cell(change.subject.position);
			tile(index).power += change.power;
		}
		break;

		case Change::Type::UPGRADED:
		{
			Cell index = cell(change.subject.position);
			tile(index) = change.tile;
			tile(index).resetId();
		}
		break;

		case Change::Type::CULTIVATES:
		{
			Cell index = cell(change.subject.position);
			tile(index).power += change.power;
		}
		break;

		case Change::Type::CULTIVATED:
		{
			Cell index = cell(change.subject.position);
			tile(index) = change.tile;
			tile(index).resetId();
		}
		break;
		case Change::Type::CAPTURES:
		break;

		case Change::Type::CAPTURED:
		{
			Cell index = cell(change.subject.position);
			tile(index).owner = change.player;
			tile(index).resetId();
		}
		break;

		case Change::Type::PRODUCES:
		{
			Cell index = cell(change.subject.position);
			tile(index).power += change.power;
		}
		break;

		case Change::Type::PRODUCED:
		{
			Cell index = cell(change.subject.position);
			unit(index, change.subject.type) = change.unit;
			unit(index, change.subject.type).resetId();
		}
		break;

		case Change::Type::ENTERED:
		{
			Cell index = cell(change.subject.position);
			unit(index, change.subject.type) = change.unit;
			unit(index, change.subject.type).resetId();
		}
		break;

		case Change::Type::EXITED:
		{
			Cell index = cell(change.subject.position);
			unit(index, change.subject.type) = UnitToken();
		}
		break;

		case Change::Type::DIED:
		{
			Cell index = cell(change.subject.position);
			unit(index, change.subject.type) = UnitToken();
		}
		break;

		case Change::Type::DESTROYED:
		{
			Cell index = cell(change.subject.position);
			tile(index) = change.tile;
			tile(index).resetId();
		}
		break;

		case Change::Type::SURVIVED:
		break;

		case Change::Type::AIMS:
		case Change::Type::ATTACKS:
		case Change::Type::TRAMPLES:
		case Change::Type::SHELLS:
		case Change::Type::BOMBARDS:
		case Change::Type::BOMBS:
		break;

		case Change::Type::ATTACKED:
		case Change::Type::TRAMPLED:
		case Change::Type::SHELLED:
		case Change::Type::BOMBARDED:
		case Change::Type::BOMBED:
		case Change::Type::FROSTBITTEN:
		case Change::Type::BURNED:
		case Change::Type::GASSED:
		case Change::Type::IRRADIATED:
		{
			Cell index = cell(change.subject.position);
			switch (change.subject.type)
			{
				// If the cell is hit, this indicates that the shot missed.
				case Descriptor::Type::CELL:
				break;
				// If a tile was hit, it might remove a stack and/or depower a powered stack.
				case Descriptor::Type::TILE:
				{
					if (change.killed)
					{
						tile(index).stacks -= 1;
					}
					if (change.depowered)
					{
						tile(index).power -= 1;
					}
				}
				break;
				// If a unit was hit, it might remove a stack.
				case Descriptor::Type::GROUND:
				case Descriptor::Type::AIR:
				case Descriptor::Type::BYPASS:
				{
					if (change.killed)
					{
						unit(index, change.subject.type).stacks -= 1;
					}
				}
				break;
				// Error.
				case Descriptor::Type::NONE:
				{
					LOGE << "cannot enact change without subject: "
						<< TypeEncoder(&_typenamer) << change;
				}
				break;
			}
		}
		break;

		case Change::Type::GROWS:
		{
			Cell index = cell(change.subject.position);
			tile(index).stacks += change.stacks;
			tile(index).power += change.power;
		}
		break;

		case Change::Type::SNOW:
		{
			Cell index = cell(change.subject.position);
			snow(index) = change.snow;
		}
		break;

		case Change::Type::FROSTBITE:
		{
			Cell index = cell(change.subject.position);
			frostbite(index) = change.frostbite;
		}
		break;

		case Change::Type::FIRESTORM:
		{
			Cell index = cell(change.subject.position);
			firestorm(index) = change.firestorm;
		}
		break;

		case Change::Type::BONEDROUGHT:
		{
			Cell index = cell(change.subject.position);
			bonedrought(index) = change.bonedrought;
		}
		break;

		case Change::Type::DEATH:
		{
			Cell index = cell(change.subject.position);
			death(index) = change.death;
		}
		break;

		case Change::Type::GAS:
		{
			Cell index = cell(change.subject.position);
			gas(index) += change.gas;
		}
		break;

		case Change::Type::RADIATION:
		{
			Cell index = cell(change.subject.position);
			radiation(index) += change.radiation;
		}
		break;

		case Change::Type::TEMPERATURE:
		{
			Cell index = cell(change.subject.position);
			temperature(index) += change.temperature;
		}
		break;

		case Change::Type::HUMIDITY:
		{
			Cell index = cell(change.subject.position);
			humidity(index) += change.humidity;
		}
		break;

		case Change::Type::CHAOS:
		{
			Cell index = cell(change.subject.position);
			chaos(index) += change.chaos;
		}
		break;

		case Change::Type::VISION:
		{
			Cell index = cell(change.subject.position);
			vision(index) = change.vision;
		}
		break;

		case Change::Type::CORNER:
		{
			int cols = ((int) change.subject.position.col) + 1;
			int rows = ((int) change.subject.position.row) + 1;

			if (cols != _cols || _rows != rows)
			{
				resize(cols, rows);
			}
		}
		break;

		case Change::Type::BORDER:
		break;

		case Change::Type::CHAOSREPORT:
		case Change::Type::YEAR:
		case Change::Type::SEASON:
		case Change::Type::DAYTIME:
		case Change::Type::PHASE:
		case Change::Type::INITIATIVE:
		case Change::Type::FUNDS:
		case Change::Type::INCOME:
		case Change::Type::EXPENDITURE:
		case Change::Type::SLEEPING:
		case Change::Type::ACTING:
		case Change::Type::FINISHED:
		case Change::Type::DISCARDED:
		case Change::Type::POSTPONED:
		case Change::Type::UNFINISHED:
		case Change::Type::ORDERED:
		case Change::Type::SCORED:
		case Change::Type::DEFEAT:
		case Change::Type::VICTORY:
		case Change::Type::GAMEOVER:
		case Change::Type::AWARD:
		case Change::Type::NONE:
		break;
	}
}
