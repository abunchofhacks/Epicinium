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

#include "vision.hpp"
#include "position.hpp"
#include "tiletoken.hpp"
#include "unittoken.hpp"
#include "descriptor.hpp"


class Space
{
public:
	friend class Board;

	Space(int8_t r, int8_t c) :
		_position(r, c)
	{}

	Space(const Space&) = delete;
	Space(Space&&) = default;
	Space& operator=(const Space&) = delete;
	Space& operator=(Space&&) = default;
	~Space() = default;

private:
	Vision _vision;

	Position _position;

	int8_t _temperature = 0;
	int8_t _humidity = 0;
	int8_t _chaos = 0;
	int8_t _gas = 0;
	int8_t _radiation = 0;

	bool _snow = false;
	bool _frostbite = false;
	bool _firestorm = false;
	bool _bonedrought = false;
	bool _death = false;

	TileTokenWithId _tile;
	UnitTokenWithId _ground;
	UnitTokenWithId _air;
	UnitTokenWithId _bypass;

public:
	bool current() const { return !_vision.empty(); }

	const Vision& vision() const { return _vision; }
	Vision& vision() { return _vision; }

	const Position& position() const { return _position; }

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
	bool firestorm()   const { return _firestorm;   }
	bool bonedrought() const { return _bonedrought; }
	bool death()       const { return _death;       }

	bool& snow()        { return _snow;        }
	bool& frostbite()   { return _frostbite;   }
	bool& firestorm()   { return _firestorm;   }
	bool& bonedrought() { return _bonedrought; }
	bool& death()       { return _death;       }

	const TileTokenWithId& tile()   const { return _tile;   }
	const UnitTokenWithId& ground() const { return _ground; }
	const UnitTokenWithId& air()    const { return _air;    }
	const UnitTokenWithId& bypass() const { return _bypass; }

	TileTokenWithId& tile()   { return _tile;   }
	UnitTokenWithId& ground() { return _ground; }
	UnitTokenWithId& air()    { return _air;    }
	UnitTokenWithId& bypass() { return _bypass; }

	const UnitTokenWithId& unit(Descriptor::Type type) const
	{
		switch (type)
		{
			case Descriptor::Type::GROUND: return ground(); break;
			case Descriptor::Type::AIR:    return air();    break;
			case Descriptor::Type::BYPASS: return bypass(); break;
			default:                       return bypass(); break;
		}
	}

	UnitTokenWithId& unit(Descriptor::Type type)
	{
		switch (type)
		{
			case Descriptor::Type::GROUND: return ground(); break;
			case Descriptor::Type::AIR:    return air();    break;
			case Descriptor::Type::BYPASS: return bypass(); break;
			default:                       return bypass(); break;
		}
	}
};
