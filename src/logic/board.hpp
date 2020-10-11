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

#include "position.hpp"
#include "cell.hpp"
#include "area.hpp"
#include "space.hpp"

struct Change;
class TypeNamer;
enum class Player : uint8_t;

class Board
{
public:
	explicit Board(const TypeNamer& typenamer);

	Board(const Board&) = delete;
	Board(Board&&) = delete;
	Board& operator=(const Board&) = delete;
	Board& operator=(Board&&) = delete;
	~Board() = default;

private:
	const TypeNamer& _typenamer;
	int _cols;
	int _rows;

	std::vector<Space> _spaces;

	std::vector<Player> _players;

	void resize(int cols, int rows);

	int checkedindex(int r, int c) const
	{
		return (r >= 0 && c >= 0 && r < _rows && c < _cols)
			? (r * _cols + c)
			: (_rows * _cols);
	}

	Space& at(const Position& pos)
	{
		return _spaces[checkedindex(pos.row, pos.col)];
	}

	const Space& at(const Position& pos) const
	{
		return _spaces[checkedindex(pos.row, pos.col)];
	}

public:
	int rows() const { return _rows; }
	int cols() const { return _cols; }

	Space& at(Cell index)
	{
		return _spaces[index.ix()];
	}

	const Space& at(Cell index) const
	{
		return _spaces[index.ix()];
	}

	void clear(int cols, int rows);
	void load(const std::string& mapname);
	void loadCellFromJson(Cell index, const Json::Value& celljson);

	int mass() const { return (rows() * cols()) / 125; }

	// Pass by value is intentional.
	void assignPlayers(std::vector<Player> players);
	void setPlayers(std::vector<Player> players);

	std::vector<Player> players();

	void enact(const Change& change);

	bool current(Cell i) const { return at(i).current(); }

	const Vision& vision(Cell i) const { return at(i).vision(); }
	Vision& vision(Cell i) { return at(i).vision(); }

	int8_t temperature(Cell i) const { return at(i).temperature(); }
	int8_t humidity(Cell i)    const { return at(i).humidity();    }
	int8_t chaos(Cell i)       const { return at(i).chaos();       }
	int8_t gas(Cell i)         const { return at(i).gas();         }
	int8_t radiation(Cell i)   const { return at(i).radiation();   }

	int8_t& temperature(Cell i) { return at(i).temperature(); }
	int8_t& humidity(Cell i)    { return at(i).humidity();    }
	int8_t& chaos(Cell i)       { return at(i).chaos();       }
	int8_t& gas(Cell i)         { return at(i).gas();         }
	int8_t& radiation(Cell i)   { return at(i).radiation();   }

	bool snow(Cell i)        const { return at(i).snow();        }
	bool frostbite(Cell i)   const { return at(i).frostbite();   }
	bool firestorm(Cell i)   const { return at(i).firestorm();   }
	bool bonedrought(Cell i) const { return at(i).bonedrought(); }
	bool death(Cell i)       const { return at(i).death();       }

	bool& snow(Cell i)        { return at(i).snow();        }
	bool& frostbite(Cell i)   { return at(i).frostbite();   }
	bool& firestorm(Cell i)   { return at(i).firestorm();   }
	bool& bonedrought(Cell i) { return at(i).bonedrought(); }
	bool& death(Cell i)       { return at(i).death();       }

	const TileTokenWithId& tile(Cell i)   const { return at(i).tile();   }
	const UnitTokenWithId& ground(Cell i) const { return at(i).ground(); }
	const UnitTokenWithId& air(Cell i)    const { return at(i).air();    }
	const UnitTokenWithId& bypass(Cell i) const { return at(i).bypass(); }

	TileTokenWithId& tile(Cell i)   { return at(i).tile();   }
	UnitTokenWithId& ground(Cell i) { return at(i).ground(); }
	UnitTokenWithId& air(Cell i)    { return at(i).air();    }
	UnitTokenWithId& bypass(Cell i) { return at(i).bypass(); }

	const UnitTokenWithId& unit(Cell i, Descriptor::Type type) const
	{
		switch (type)
		{
			case Descriptor::Type::GROUND: return ground(i); break;
			case Descriptor::Type::AIR:    return air(i);    break;
			case Descriptor::Type::BYPASS: return bypass(i); break;
			default:                       return bypass(i); break;
		}
	}

	UnitTokenWithId& unit(Cell i, Descriptor::Type type)
	{
		switch (type)
		{
			case Descriptor::Type::GROUND: return ground(i); break;
			case Descriptor::Type::AIR:    return air(i);    break;
			case Descriptor::Type::BYPASS: return bypass(i); break;
			default:                       return bypass(i); break;
		}
	}

	Cell cell(const Position& pos) const
	{
		return Cell::create(_rows, _cols, checkedindex(pos.row, pos.col));
	}

	Cell begin() const
	{
		return Cell::create(_rows, _cols, 0);
	}

	Cell end() const
	{
		// Note that end() is exclusive and hence _spaces[_rows * _cols] is not
		// included when doing a range-based for loop over Board.
		return Cell::create(_rows, _cols, _rows * _cols);
	}

	Cell cells() const
	{
		return end();
	}

	Area area(Cell centrum, int min, int max) const
	{
		return Area(centrum, min, max);
	}

	Area area(const Position& centrum, int min, int max) const
	{
		return Area(cell(centrum), min, max);
	}
};
