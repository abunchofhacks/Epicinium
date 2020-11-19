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
#include "pathingfloodfill.hpp"
#include "source.hpp"

#include "bible.hpp"
#include "board.hpp"


template <class This>
void PathingFI<This>::flood(Cell from)
{
	for (const Move& move : {Move::E, Move::S, Move::W, Move::N})
	{
		Cell to = from + move;
		TileType tiletype = _board.tile(to).type;

		if (tiletype != TileType::NONE
			&& _bible.tileAccessible(tiletype)
			&& (_air || _bible.tileWalkable(tiletype)))
		{
			put(from, to);
		}
	}
}

template <class This>
Move PathingFI<This>::step(Cell from) const
{
	Move result = Move::X;
	uint16_t smallest = get(from);

	// shuffle to prevent congestion (sometimes...)
	std::vector<Move> moves = {Move::E, Move::S, Move::W, Move::N};
	std::random_shuffle(moves.begin(), moves.end());

	for (const Move& move : moves)
	{
		Cell to = from + move;
		TileType tiletype = _board.tile(to).type;

		if (tiletype != TileType::NONE && get(to) < smallest)
		{
			result = move;
			smallest = get(to);
		}
	}
	return result;
}

template <class This>
uint16_t PathingFI<This>::steps(Cell at) const
{
	return get(at);
}

template <class This>
bool PathingFI<This>::reached(Cell at) const
{
	return (get(at) != uint16_t(-1));
}

void TileFloodfill::map(Cell index)
{
	TileType tiletype = _board.tile(index).type;

	// TODO unnecessary?
	if (tiletype == TileType::NONE) return;

	Player tileowner = _board.tile(index).owner;
	bool playerfound = (std::find(_players.begin(), _players.end(), tileowner)
		!= _players.end());
	if (_excludePlayers && playerfound) return;
	if (!_excludePlayers && !playerfound) return;

	bool typefound = (std::find(_types.begin(), _types.end(), tiletype)
		!= _types.end());
	if (_excludeTypes && typefound) return;
	if (!_excludeTypes && !typefound) return;

	if (_excludeOccupied)
	{
		UnitType unittype = _board.ground(index).type;
		if (unittype != UnitType::NONE
			&& _board.ground(index).owner != tileowner
			&& _bible.unitCanOccupy(unittype)) return;
	}


	if (_includeOccupied)
	{
		UnitType unittype = _board.ground(index).type;
		if (unittype == UnitType::NONE
			|| _board.ground(index).owner == tileowner
			|| !(_bible.unitCanOccupy(unittype))) return;
	}

	put(index);
}

void TileFloodfill::include(std::vector<Player> players)
{
	_players = players;
	_excludePlayers = false;
}

void TileFloodfill::exclude(std::vector<Player> players)
{
	_players = players;
	_excludePlayers = true;
}

void TileFloodfill::include(std::vector<TileType> types)
{
	_types = types;
	_excludeTypes = false;
}

void TileFloodfill::exclude(std::vector<TileType> types)
{
	_types = types;
	_excludeTypes = true;
}

void TileFloodfill::excludeOccupied(bool exclude)
{
	_excludeOccupied = exclude;
}

void TileFloodfill::includeOccupied(bool include)
{
	_includeOccupied = include;
}

void UnitFloodfill::map(Cell index)
{
	UnitType unittype = _board.ground(index).type;

	// TODO unnecessary?
	if (unittype == UnitType::NONE) return;

	Player unitowner = _board.ground(index).owner;
	bool playerfound = (std::find(_players.begin(), _players.end(), unitowner)
		!= _players.end());
	if (_excludePlayers && playerfound) return;
	if (!_excludePlayers && !playerfound) return;

	bool typefound = (std::find(_types.begin(), _types.end(), unittype)
		!= _types.end());
	if (_excludeTypes && typefound) return;
	if (!_excludeTypes && !typefound) return;

	put(index);
}

void UnitFloodfill::include(std::vector<Player> players)
{
	_players = players;
	_excludePlayers = false;
}

void UnitFloodfill::exclude(std::vector<Player> players)
{
	_players = players;
	_excludePlayers = true;
}

void UnitFloodfill::include(std::vector<UnitType> types)
{
	_types = types;
	_excludeTypes = false;
}

void UnitFloodfill::exclude(std::vector<UnitType> types)
{
	_types = types;
	_excludeTypes = true;
}

template class PathingFI<PathingFloodfill>;
template class PathingFI<TileFloodfill>;
template class PathingFI<UnitFloodfill>;
