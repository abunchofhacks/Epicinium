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
#include "vision.hpp"
#include "tile.hpp"
#include "unit.hpp"
#include "descriptor.hpp"
#include "move.hpp"
#include "area.hpp"
#include "cell.hpp"

class PlacementGrid;
class PlacementBox;
struct PlacementPather;
class Bible;
class Skinner;
enum class TreeType : uint8_t;


class Square
{
public:
	friend class Level;

	Square(Square* grid, int8_t rows, int8_t cols,
		const Bible& bible, const Skinner& skinner,
		int8_t r, int8_t c);
	Square(const Square&) = delete;
	Square(Square&& that);
	Square& operator=(const Square&) = delete;
	Square& operator=(Square&&) = delete;
	~Square();

	int16_t edgeindex() const;
	int16_t index(int r, int c) const;
	int16_t checkedindex(int r, int c) const;

	Square* grid() const { return _grid; }
	int gridrows() const { return _rows; }
	int gridcols() const { return _cols; }

private:
	const Bible& _bible;
	const Skinner& _skinner;

	// TODO cleanup (#948, #1191)
	Square* _grid;
	int8_t _rows;
	int8_t _cols;

	bool _real;
	Vision _vision;

	Position _position;
	int16_t _index;

	int16_t _east;
	int16_t _south;
	int16_t _west;
	int16_t _north;

	int8_t _temperature = 0;
	int8_t _humidity = 0;
	int8_t _chaos = 0;
	int8_t _gas = 0;
	int8_t _radiation = 0;

	bool _snow = false;
	bool _frostbite = false;
	bool _coldfeet = false;
	bool _firestorm = false;
	bool _bonedrought = false;
	bool _death = false;

	// Note that _placementgrid needs to be destroyed after _tile, _ground, _air
	// and _bypass are destroyed.
	std::unique_ptr<PlacementGrid> _placementgrid;

	Tile _tile;
	Unit _ground;
	Unit _air;
	Unit _bypass;

	float _light;
	float _spotlight;
	float _flashlight;

public:
	bool edge()    const { return !_real;   }
	bool current() const { return !_vision.empty(); }

	const Vision& vision() const { return _vision; }
	Vision& vision() { return _vision; }

	const Position& position() const { return _position; }

	const Square* east()  const { return _grid + _east;  }
	const Square* south() const { return _grid + _south; }
	const Square* west()  const { return _grid + _west;  }
	const Square* north() const { return _grid + _north; }

	Square* east()  { return _grid + _east;  }
	Square* south() { return _grid + _south; }
	Square* west()  { return _grid + _west;  }
	Square* north() { return _grid + _north; }

	const Square* eswn(Move move) const;
	Square* eswn(Move move);

	int8_t temperature() const { return _temperature; }
	int8_t humidity()    const { return _humidity;    }
	int8_t chaos()       const { return _chaos;       }
	int8_t gas()         const { return _gas;         }
	int8_t radiation()   const { return _radiation;   }

	int8_t& temperature() { return _temperature; }
	int8_t& humidity()    { return _humidity;    }
	int8_t& chaos()       { return _chaos;       }
	int8_t& gas()         { return _gas;         }
	int8_t& radiation()   { return _radiation;   }

	bool snow()        const { return _snow;        }
	bool frostbite()   const { return _frostbite;   }
	bool coldfeet()    const { return _coldfeet;   }
	bool firestorm()   const { return _firestorm;   }
	bool bonedrought() const { return _bonedrought; }
	bool death()       const { return _death;       }

	bool& snow()        { return _snow;        }
	bool& frostbite()   { return _frostbite;   }
	bool& coldfeet()    { return _coldfeet;   }
	bool& firestorm()   { return _firestorm;   }
	bool& bonedrought() { return _bonedrought; }
	bool& death()       { return _death;       }

	const Tile& tile()   const { return _tile;   }
	const Unit& ground() const { return _ground; }
	const Unit& air()    const { return _air;    }
	const Unit& bypass() const { return _bypass; }

	Tile& tile()   { return _tile;   }
	Unit& ground() { return _ground; }
	Unit& air()    { return _air;    }
	Unit& bypass() { return _bypass; }

	const Unit& unit(Descriptor::Type type) const;
	Unit& unit(Descriptor::Type type);

	const Skinner& skinner() const { return _skinner; }

	PlacementBox place(Descriptor::Type type);
	PlacementBox place(Descriptor::Type type,
		const PlacementBox& oldbox, PlacementPather& pather);

	void spawn(PlacementBox& box,
		uint8_t buildingindex, PlacementPather& pather);

	float light() { return _light; }
	void setLight(float light, float duration = 0, float delay = 0);
	float spotlight() { return _spotlight; }
	void setSpotlight(float light, float duration = 0, float delay = 0);
	float flashlight() { return _flashlight; }
	void setFlashlight(float light, float duration = 0, float delay = 0);

	void update();
	void cleanup();

	// TODO cleanup (#948, #1191)
	Area area(int min, int max) const
	{
		return Area(cell(), min, max);
	}

	Cell cell() const
	{
		return Cell::create(_rows, _cols,
			checkedindex(_position.row, _position.col));
	}

	const Square* get(const Position& pos) const
	{
		return _grid + checkedindex(pos.row, pos.col);
	}

	Square* get(const Position& pos)
	{
		return _grid + checkedindex(pos.row, pos.col);
	}
};
