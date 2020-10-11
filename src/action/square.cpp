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
#include "square.hpp"
#include "source.hpp"

#include "surface.hpp"
#include "placementgrid.hpp"
#include "placementpather.hpp"
#include "move.hpp"

int16_t Square::edgeindex() const
{
	return _rows * _cols;
}

int16_t Square::index(int r, int c) const
{
	return r * _cols + c;
}

int16_t Square::checkedindex(int r, int c) const
{
	return (r >= 0 && c >= 0 && r < _rows && c < _cols)
		? index(r, c)
		: edgeindex();
}

Square::Square(Square* grid, int8_t rows, int8_t cols,
		const Bible& bible, const Skinner& skinner,
		int8_t r, int8_t c) :
	_bible(bible),
	_skinner(skinner),
	_grid(grid),
	_rows(rows),
	_cols(cols),
	_real(checkedindex(r, c) != edgeindex()),
	_position(r, c),
	_index(checkedindex(r, c)),
	_east(checkedindex(r, c + 1)),
	_south(checkedindex(r + 1, c)),
	_west(checkedindex(r, c - 1)),
	_north(checkedindex(r - 1, c)),
	_placementgrid(nullptr),
	_tile(this, _bible, _skinner),
	_ground(this, _bible, _skinner),
	_air(this, _bible, _skinner),
	_bypass(this, _bible, _skinner),
	_light(0),
	_spotlight(1),
	_flashlight(0)
{
	Point topleft = Surface::convertTopleft(_position);
	_placementgrid.reset(new PlacementGrid(Point(topleft.xenon + 5,
				topleft.yahoo + 4),
			Point(topleft.xenon + Surface::WIDTH - 5,
				topleft.yahoo + Surface::HEIGHT - 1)));
}

Square::Square(Square&&) = default;

Square::~Square() = default;

void Square::update()
{
	_tile.update();
	_ground.update();
	_air.update();
	_bypass.update();
}

void Square::setLight(float light, float duration, float delay)
{
	_light = light;
	_tile.light(light, duration, delay);
	_ground.light(light, duration, delay);
	_air.light(light, duration, delay);
	_bypass.light(light, duration, delay);
}

void Square::setSpotlight(float light, float duration, float delay)
{
	_spotlight = light;
	_tile.spotlight(light, duration, delay);
	_ground.spotlight(light, duration, delay);
	_air.spotlight(light, duration, delay);
	_bypass.spotlight(light, duration, delay);
}

void Square::setFlashlight(float light, float duration, float delay)
{
	_flashlight = light;
	_tile.flashlight(light, duration, delay);
	_ground.flashlight(light, duration, delay);
	_air.flashlight(light, duration, delay);
	_bypass.flashlight(light, duration, delay);
}

const Square* Square::eswn(Move move) const
{
	switch (move)
	{
		case Move::X: return this;
		case Move::E: return east();
		case Move::S: return south();
		case Move::W: return west();
		case Move::N: return north();
	}
	return nullptr;
}

Square* Square::eswn(Move move)
{
	switch (move)
	{
		case Move::X: return this;
		case Move::E: return east();
		case Move::S: return south();
		case Move::W: return west();
		case Move::N: return north();
	}
	return nullptr;
}

const Unit& Square::unit(Descriptor::Type type) const
{
	switch (type)
	{
		case Descriptor::Type::GROUND: return ground(); break;
		case Descriptor::Type::AIR:    return air();    break;
		case Descriptor::Type::BYPASS: return bypass(); break;
		default:                       return bypass(); break;
	}
}

Unit& Square::unit(Descriptor::Type type)
{
	switch (type)
	{
		case Descriptor::Type::GROUND: return ground(); break;
		case Descriptor::Type::AIR:    return air();    break;
		case Descriptor::Type::BYPASS: return bypass(); break;
		default:                       return bypass(); break;
	}
}

PlacementBox Square::place(Descriptor::Type type)
{
	return _placementgrid->place(type);
}

PlacementBox Square::place(Descriptor::Type type,
		const PlacementBox& oldbox, PlacementPather& pather)
{
	auto& othergrid = eswn(flip(pather.direction))->_placementgrid;
	return _placementgrid->place(type, oldbox, pather, *othergrid);
}

void Square::spawn(PlacementBox& box,
		uint8_t buildingindex, PlacementPather& pather)
{
	auto& othergrid = eswn(flip(pather.direction))->_placementgrid;
	_placementgrid->spawnPathing(box, buildingindex, pather, *othergrid);
}

void Square::cleanup()
{
	_tile.cleanup();
	_ground.cleanup();
	_air.cleanup();
	_bypass.cleanup();
}
