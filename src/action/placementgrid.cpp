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
 * Can Ur (canur11@gmail.com)
 */
#include "placementgrid.hpp"
#include "source.hpp"

#include <bitset>

#include "surface.hpp"
#include "placementpather.hpp"
#include "mixer.hpp"

/*
Generating the placements of the tile figures is bound to four rules:
The Visibility Rule:   a tile figure may not obscure another tile figure or unit figure;
The Readability Rule:  a tile or unit figure may not be obscured by the guide stamp;
The Sudoku Rule:       two tile figures can't be placed in the same row or column within a grid;
The No-Clipping Rule:  two tile figures can't be adjacent in the same row across grid borders.

The Visibility Rule prevents figures from simply not being visible to the player.
The Readability Rule prevents the guide stamp from losing readability.
The Sudoku Rule is more aesthetical; it makes forests, cities and mountains look more erratic.
The No-Clipping Rule is relevant for industry and mountains as they are wider than their placement box.

Achieving the Sudoku Rule is easy: we put each column index in a bag and then randomly
pair them with row indices. Satisfying the Visibility Rule then becomes a matter of crossing off
two space per tile placement instead of one. One problem: we need to know where the tile
on the surface below us will be placement to ensure we don't place a tile there.
Therefore the top-most tile placement is deterministic given a PlacementGrid's position.

For the Readability Rule we must prevent the top-most and bottom-most tile
figures from being placed in the left-most position. For the No-Clipping Rule,
we need to make the row index of the left-most tile placement
deterministic. We then ensure that the tile placement in the same row as our right neighbor's
left-most tile placement is not our right-most tile placement.
*/

static constexpr uint8_t GRIDSIZE = 5;

static constexpr uint8_t tower(uint8_t r, uint8_t c)
{
	return 1 + (r + 3 * c) % 4;
}

static constexpr uint8_t bridge(uint8_t r, uint8_t c)
{
	return 1 + (2 * r + c) % 3;
}

PlacementGrid::PlacementGrid(const Point& topleft, const Point& bottomright) :
	_topleft(topleft),
	_bottomright(bottomright),
	_width(_bottomright.xenon - _topleft.xenon),
	_height(_bottomright.yahoo - _topleft.yahoo),
	_boxwidth(_width / GRIDSIZE),
	_boxheight(_height / GRIDSIZE),
	_tileplacements1(new PlacementLayer()),
	_tileplacements2(new PlacementLayer()),
	_unitplacements1(new PlacementLayer()),
	_unitplacements2(new PlacementLayer()),
	_airplacements(new PlacementLayer()),
	_fenceplacements(new PlacementLayer())
{
	// We will cross off certain spaces as being reserved for tile figures or air figures.
	// The remaining spaces can be used by ground unit figures.
	std::bitset<GRIDSIZE * GRIDSIZE> placements;
	placements.set();

	// Due to city buildings being larger than before, we prefer not to use the
	// *two* spaces above them for ground units.
	std::bitset<GRIDSIZE * GRIDSIZE> warnings;

	// The guide stamp obscures a space on our grid.
	placements.reset((GRIDSIZE - 1) * GRIDSIZE + 0);

	// Determine the positioning of our tower, the tower of the surface below us,
	// our bridge and the bridge of the surface to our right.
	uint8_t mapcol = (_topleft.xenon > 0) ? _topleft.xenon / Surface::WIDTH
		: (_topleft.xenon / Surface::WIDTH - 1);
	uint8_t maprow = (_topleft.yahoo > 0) ? _topleft.yahoo / Surface::HEIGHT
		: (_topleft.yahoo / Surface::HEIGHT - 1);
	// Because tower() and bridge() repeat themselves after 4 and 3 (resp.),
	// we want maprow -1 = 255 to be treated as maprow 251 = 21 * 12 - 1.
	if (mapcol >= 200) mapcol -= (256 - 252);
	if (maprow >= 200) maprow -= (256 - 252);
	uint8_t ourtower = tower(maprow, mapcol);
	uint8_t theirtower = tower(maprow + 1, mapcol);
	uint8_t ourbridge = bridge(maprow, mapcol);
	uint8_t theirbridge = bridge(maprow, mapcol + 1);

	// Add a tileplacement for the top row, i.e. our "tower".
	_possibletileplacements.emplace_back(ourtower);
	_tileplacements2->push(ourtower);
	placements.reset(ourtower);

	// The top half of the tower below us obscures a space on our grid.
	placements.reset((GRIDSIZE - 1) * GRIDSIZE + theirtower);
	warnings.set((GRIDSIZE - 2) * GRIDSIZE + theirtower);

	// We might also want to set a warning for the space above the second
	// highest building in the space below us, but we don't know where that
	// will be placed. However, we generally don't want to place unit figures
	// on the bottom row anyway.
	for (uint8_t c = 0; c < GRIDSIZE; c++)
	{
		warnings.set((GRIDSIZE - 1) * GRIDSIZE + c);
	}

	// Determine the column of the bottom tower.
	Randomizer<uint8_t> cplacements;
	bool first = true;
	for (uint8_t c = 0; c < GRIDSIZE; c++, first = false)
	{
		// It cannot be the same column as our tower (because of The Sudoku Rule).
		if (c == ourtower) continue;
		// It cannot be the same column as the tower below us (because of The Visiblity Rule).
		if (c == theirtower) continue;
		// It cannot be the first column, because that is reserved for our bridge.
		if (first) continue;
		// Other columns are OK.
		cplacements.push(c);
	}
	uint8_t bottower = cplacements.pop();

	// Add a tileplacement for the bottom row.
	{
		uint8_t r = GRIDSIZE - 1;
		uint8_t c = bottower;
		_possibletileplacements.emplace_back(r * GRIDSIZE + c);
		_tileplacements2->push(r * GRIDSIZE + c);
		placements.reset(r * GRIDSIZE + c);
		// Reserve the space above it (Visiblity Rule).
		placements.reset((r - 1) * GRIDSIZE + c);
		if (r >= 2) warnings.set((r - 2) * GRIDSIZE + c);
	}

	// Add a fence placement below the bottom tower.
	{
		uint8_t r = GRIDSIZE - 1;
		uint8_t c = bottower;
		_fenceplacements->push(r * GRIDSIZE + c);
	}

	// Determine the columns for the other tileplacements.
	cplacements.clear();
	uint8_t ourbridgecol = 255;
	uint8_t theirbridgecol = 255;
	for (uint8_t c = 0; c < GRIDSIZE; c++)
	{
		// It cannot be the same as our tower or the bottom tileplacement (Sudoku Rule).
		if (c == ourtower || c == bottower) continue;
		// The first available column is reserved for our bridge.
		if (ourbridgecol == 255)
		{
			ourbridgecol = c;
			continue;
		}
		// The second available column is reserved for a tileplacement in the same row
		// as the bridge to our right (in order to satisfy The No-Clipping Rule).
		if (theirbridgecol == 255)
		{
			theirbridgecol = c;
			continue;
		}
		// Other columns are OK.
		cplacements.push(c);
	}

	// If ourtower is in the top-left and bottower is in the bottom-right,
	// we want to avoid a complete diagonal because that looks "unrandom".
	// Since the left-most and right-most buildings are known,
	// we are at complete liberty to randomize the middle three columns.
	if (ourtower == 0 && bottower == GRIDSIZE - 1)
	{
		cplacements.push(ourbridgecol);
		cplacements.push(theirbridgecol);
	}

	// Add tileplacements for the remaining rows.
	for (uint8_t r = 1; r < GRIDSIZE - 1; r++)
	{
		uint8_t c;

		if (ourtower == 0 && bottower == GRIDSIZE - 1)
		{
			c = cplacements.pop();
			while (c == r && !cplacements.empty())
			{
				uint8_t oldc = c;
				c = cplacements.pop();
				cplacements.push(oldc);
			}
		}
		else if (r == ourbridge  ) c = ourbridgecol;
		else if (r == theirbridge) c = theirbridgecol;
		else                       c = cplacements.pop();

		_possibletileplacements.emplace_back(r * GRIDSIZE + c);
		if (c > 0 && c < GRIDSIZE - 1)
		{
			_tileplacements1->push(r * GRIDSIZE + c);
		}
		else
		{
			_tileplacements2->push(r * GRIDSIZE + c);
		}
		placements.reset(r * GRIDSIZE + c);
		// Reserve the space above it (Visiblity Rule).
		placements.reset((r - 1) * GRIDSIZE + c);
		if (r >= 2) warnings.set((r - 2) * GRIDSIZE + c);
	}

	// Zeppelins are always parked in the center.
	uint8_t middle = GRIDSIZE / 2;
	_airplacements->push(middle * GRIDSIZE + middle);
	// Added twice to allow air units to crash into eachother.
	_airplacements->push(middle * GRIDSIZE + middle);
	// Visually, this means they obscure the middle of the top row.
	// Zeppelins may obscure tile figures but not unit figures.
	// Zeppelins can either be horizontal or vertical, but due to perspective
	// they are very flat, so we only need to reserve three placements.
	placements.reset(middle - 1);
	placements.reset(middle);
	placements.reset(middle + 1);

	// Add unitplacements for all the remaining spaces.
	for (uint8_t r = 0; r < GRIDSIZE; r++)
	{
		for (uint8_t c = 0; c < GRIDSIZE; c++)
		{
			if (placements[r * GRIDSIZE + c])
			{
				_possibleunitplacements.emplace_back(r * GRIDSIZE + c);
				if (   (r > 0 && r < GRIDSIZE - 1)
					&& (c > 0 && c < GRIDSIZE - 1)
					&& !warnings[r * GRIDSIZE + c])
				{
					_unitplacements1->push(r * GRIDSIZE + c);
				}
				else
				{
					_unitplacements2->push(r * GRIDSIZE + c);
				}
			}
		}
	}
}

PlacementBox PlacementGrid::place(Descriptor::Type owner)
{
	switch (owner)
	{
		case Descriptor::Type::NONE:
		case Descriptor::Type::GROUND:
		case Descriptor::Type::BYPASS:
		{
			if (!_unitplacements1->empty()) return get(_unitplacements1);
			else return get(_unitplacements2);
		}
		break;

		case Descriptor::Type::TILE:
		{
			if (!_tileplacements1->empty()) return get(_tileplacements1);
			else return get(_tileplacements2);
		}
		break;

		case Descriptor::Type::AIR:
		{
			return get(_airplacements);
		}
		break;

		case Descriptor::Type::CELL:
		{
			return get(_fenceplacements);
		}
		break;
	}
	return get(_unitplacements2);
}

PlacementBox PlacementGrid::place(Descriptor::Type owner,
		const PlacementBox& old, PlacementPather& pather, PlacementGrid& other)
{
	switch (owner)
	{
		case Descriptor::Type::NONE:
		case Descriptor::Type::GROUND:
		case Descriptor::Type::BYPASS:
		{
			if (!_unitplacements1->empty())
			{
				return get(_unitplacements1, old._index, pather, other);
			}
			else if (!_unitplacements2->empty())
			{
				return get(_unitplacements2, old._index, pather, other);
			}
		}
		break;

		case Descriptor::Type::TILE:
		case Descriptor::Type::AIR:
		case Descriptor::Type::CELL:
		break;
	}

	return place(owner);
}

void PlacementGrid::spawnPathing(PlacementBox& box, uint8_t buildingindex,
		PlacementPather& pather, PlacementGrid& other)
{
	return addPathing(box, buildingindex, true, pather, other);
}

PlacementBox PlacementGrid::get(const std::shared_ptr<PlacementLayer> layer)
{
	if (!(*layer))
	{
		LOGE << "no more placements in PlacementGrid::place";
		return PlacementBox(nullptr, 255, _topleft, _bottomright);
	}

	uint8_t index = layer->pop();

	return get(layer, index);
}

PlacementBox PlacementGrid::get(const std::shared_ptr<PlacementLayer> layer,
		uint8_t index)
{
	// If _width is not an exact multiple of _boxwidth, we make sure the PlacementBoxes are
	// evenly spread out over the entire surface by calculating the relative dx and dy.
	uint8_t row = index / GRIDSIZE;
	uint8_t col = index % GRIDSIZE;
	int dx = _width  * col / GRIDSIZE;
	int dy = _height * row / GRIDSIZE;
	Point tlc = Point(_topleft.xenon + dx, _topleft.yahoo + dy);
	Point brc = Point(tlc.xenon + _boxwidth, tlc.yahoo + _boxheight);

	return PlacementBox(layer, index, tlc, brc);
}

PlacementBox PlacementGrid::get(const std::shared_ptr<PlacementLayer> layer,
		uint8_t oldindex, PlacementPather& pather, PlacementGrid& other)
{
	if (!(*layer))
	{
		LOGE << "no more placements in PlacementGrid::place";
		return PlacementBox(nullptr, 255, _topleft, _bottomright);
	}

	// Instead of picking a placement at random, we will select the optimal one.
	size_t found = 0;
	size_t mindiff = 5;

	if (pather.direction == Move::E || pather.direction == Move::W)
	{
		// Find a placement in the same row, or as close as possible.
		uint8_t oldrow = oldindex / GRIDSIZE;
		for (size_t i = 0; i < layer->count(); i++)
		{
			uint8_t index = layer->inspect(i);
			uint8_t row = index / GRIDSIZE;
			uint8_t diff = abs(int(row - oldrow));
			if (diff <= mindiff)
			{
				found = i;
				mindiff = diff;
			}
		}
	}
	else
	{
		// Find a placement in the same column, or as close as possible.
		uint8_t oldcol = oldindex % GRIDSIZE;
		for (size_t i = 0; i < layer->count(); i++)
		{
			uint8_t index = layer->inspect(i);
			uint8_t col = index % GRIDSIZE;
			uint8_t diff = abs(int(col - oldcol));
			if (diff <= mindiff)
			{
				found = i;
				mindiff = diff;
			}
		}
	}

	// Select the placement we found, so that the next pop is rigged.
	layer->select(found);
	uint8_t newindex = layer->pop();
	PlacementBox box = get(layer, newindex);

	// We add pathing from the old placement to the new placement.
	addPathing(box, oldindex, false, pather, other);

	return box;
}

void PlacementGrid::addPathing(PlacementBox& box,
		uint8_t oldindex, bool spawn,
		PlacementPather& pather, PlacementGrid& other)
{
	// Because figures walk from one square to the next, they change grids.
	// Therefore we need to stitch two grids together. We first calculate the
	// size and relative position of our grid (0) and the other grid (1).
	// Note that we are the new grid and the other grid is the old grid.
	uint8_t rs = GRIDSIZE;
	uint8_t cs = GRIDSIZE;
	uint8_t r0 = 0;
	uint8_t c0 = 0;
	uint8_t r1 = 0;
	uint8_t c1 = 0;

	switch (pather.direction)
	{
		case Move::X:
		{
			// When calculating spawn pathing, unit figures spawn just below the
			// buildings that produced them. Because the building figures might
			// be at the bottom of the square, we need to add one extra row.
			rs = GRIDSIZE + spawn;
		}
		break;
		case Move::E:
		{
			cs = 2 * GRIDSIZE;
			c0 = GRIDSIZE;
		}
		break;
		case Move::S:
		{
			rs = 2 * GRIDSIZE;
			r0 = GRIDSIZE;
		}
		break;
		case Move::W:
		{
			cs = 2 * GRIDSIZE;
			c1 = GRIDSIZE;
		}
		break;
		case Move::N:
		{
			rs = 2 * GRIDSIZE;
			r1 = GRIDSIZE;
		}
		break;
	}

	// We abuse the weird perspective and proportions of the game a little here:
	// the perspective means the footprint of a figure is half as high as wide,
	// but the squares are still square. Therefore there is plenty of room
	// between two rows for figures to walk. We divide each box into two halves.
	uint8_t ys = 2 * rs;
	std::vector<uint8_t> open(ys * cs, true);
	std::vector<uint8_t> dist(ys * cs, 255);
	std::vector<uint8_t> used(ys * cs, false);

	// Because we don't remember what figures are placed where, we just look
	// at which of the original placements are unoccupied.
	// Tile figures are placed in the top half and unit figures in the bottom.
	{
		for (uint8_t index : _possibletileplacements)
		{
			uint8_t r = r0 + (index / GRIDSIZE);
			uint8_t c = c0 + (index % GRIDSIZE);
			open[(2 * r + 1) * cs + c] = false;
		}
		for (size_t i = 0; i < _tileplacements1->count(); i++)
		{
			uint8_t index = _tileplacements1->inspect(i);
			uint8_t r = r0 + (index / GRIDSIZE);
			uint8_t c = c0 + (index % GRIDSIZE);
			open[(2 * r + 1) * cs + c] = true;
		}
		for (size_t i = 0; i < _tileplacements2->count(); i++)
		{
			uint8_t index = _tileplacements2->inspect(i);
			uint8_t r = r0 + (index / GRIDSIZE);
			uint8_t c = c0 + (index % GRIDSIZE);
			open[(2 * r + 1) * cs + c] = true;
		}
		for (uint8_t index : _possibleunitplacements)
		{
			uint8_t r = r0 + (index / GRIDSIZE);
			uint8_t c = c0 + (index % GRIDSIZE);
			open[(2 * r) * cs + c] = false;
		}
		for (size_t i = 0; i < _unitplacements1->count(); i++)
		{
			uint8_t index = _unitplacements1->inspect(i);
			uint8_t r = r0 + (index / GRIDSIZE);
			uint8_t c = c0 + (index % GRIDSIZE);
			open[(2 * r) * cs + c] = true;
		}
		for (size_t i = 0; i < _unitplacements2->count(); i++)
		{
			uint8_t index = _unitplacements2->inspect(i);
			uint8_t r = r0 + (index / GRIDSIZE);
			uint8_t c = c0 + (index % GRIDSIZE);
			open[(2 * r) * cs + c] = true;
		}
	}

	if (&other != this)
	{
		for (uint8_t index : other._possibletileplacements)
		{
			uint8_t r = r1 + (index / GRIDSIZE);
			uint8_t c = c1 + (index % GRIDSIZE);
			open[(2 * r + 1) * cs + c] = false;
		}
		for (size_t i = 0; i < other._tileplacements1->count(); i++)
		{
			uint8_t index = other._tileplacements1->inspect(i);
			uint8_t r = r1 + (index / GRIDSIZE);
			uint8_t c = c1 + (index % GRIDSIZE);
			open[(2 * r + 1) * cs + c] = true;
		}
		for (size_t i = 0; i < other._tileplacements2->count(); i++)
		{
			uint8_t index = other._tileplacements2->inspect(i);
			uint8_t r = r1 + (index / GRIDSIZE);
			uint8_t c = c1 + (index % GRIDSIZE);
			open[(2 * r + 1) * cs + c] = true;
		}
		for (uint8_t index : other._possibleunitplacements)
		{
			uint8_t r = r1 + (index / GRIDSIZE);
			uint8_t c = c1 + (index % GRIDSIZE);
			open[(2 * r) * cs + c] = false;
		}
		for (size_t i = 0; i < other._unitplacements1->count(); i++)
		{
			uint8_t index = other._unitplacements1->inspect(i);
			uint8_t r = r1 + (index / GRIDSIZE);
			uint8_t c = c1 + (index % GRIDSIZE);
			open[(2 * r) * cs + c] = true;
		}
		for (size_t i = 0; i < other._unitplacements2->count(); i++)
		{
			uint8_t index = other._unitplacements2->inspect(i);
			uint8_t r = r1 + (index / GRIDSIZE);
			uint8_t c = c1 + (index % GRIDSIZE);
			open[(2 * r) * cs + c] = true;
		}
	}

	// Figures of the same unit should not avoid each other (until #1180).
	for (uint8_t index : pather.indices)
	{
		uint8_t r = r0 + (index / GRIDSIZE);
		uint8_t c = c0 + (index % GRIDSIZE);
		open[(2 * r) * cs + c] = true;
	}

	// Determine the position of the origin and target in the stitched grid.
	uint8_t newindex = box._index;
	uint8_t oldr = r1 + (oldindex / GRIDSIZE);
	uint8_t oldy = 2 * oldr + spawn * (1 + 1);
	uint8_t oldc = c1 + (oldindex % GRIDSIZE);
	uint8_t newr = r0 + (newindex / GRIDSIZE);
	uint8_t newy = 2 * newr;
	uint8_t newc = c0 + (newindex % GRIDSIZE);

	// Figures of the same unit should not avoid each other (until #1180).
	pather.indices.emplace_back(newindex);

	// Using a floodfill algorithm we will determine the shortest path from the
	// origin to the target, taking into account that moving vertically (from
	// one half to another) is twice as fast as moving horizontally. The unit
	// of time does not really matter, only the relative timing.
	std::vector<uint8_t> record;
	{
		std::vector<uint8_t> q0;
		std::vector<uint8_t> q1;
		std::vector<uint8_t> q2;

		// We do a floodfill, starting at the origin. Moving horizontally
		// takes 2 time units, so we add it to a delayed queue: q2.
		uint8_t time = 0;
		q0.push_back(oldy * cs + oldc);
		while (!q0.empty() || !q1.empty() /*|| !q2.empty()*/)
		{
			for (uint8_t yc : q0)
			{
				dist[yc] = std::min(time, dist[yc]);

				uint8_t y = yc / cs;
				uint8_t c = yc % cs;
				if (c > 0)
				{
					uint8_t uv = y * cs + (c - 1);
					if (open[uv] && dist[uv] == 255) q2.push_back(uv);
				}
				if (c + 1 < cs)
				{
					uint8_t uv = y * cs + (c + 1);
					if (open[uv] && dist[uv] == 255) q2.push_back(uv);
				}
				if (y > 0)
				{
					uint8_t uv = (y - 1) * cs + c;
					if (open[uv] && dist[uv] == 255) q1.push_back(uv);
				}
				if (y + 1 < ys)
				{
					uint8_t uv = (y + 1) * cs + c;
					if (open[uv] && dist[uv] == 255) q1.push_back(uv);
				}
			}
			q0.clear();

			q1.erase(std::unique(q1.begin(), q1.end()), q1.end());
			q2.erase(std::unique(q2.begin(), q2.end()), q2.end());

			// Assign q0 <= q1, q1 <= q2 and q2 <= {}.
			q0.swap(q1);
			q1.swap(q2);

			time++;
		}

		// Now that all the queues are cleared, we reuse them to determine a
		// shortest path. Starting at the target we work our way back to the
		// origin. We have to reverse the record once we are done.
		q0.push_back(newy * cs + newc);
		while (!q0.empty())
		{
			for (uint8_t yc : q0)
			{
				used[yc] = true;
				record.push_back(yc);

				uint8_t y = yc / cs;
				uint8_t c = yc % cs;
				if (c > 0)
				{
					uint8_t uv = y * cs + (c - 1);
					if (!used[uv] && dist[uv] < dist[yc]) q1.push_back(uv);
				}
				if (c + 1 < cs)
				{
					uint8_t uv = y * cs + (c + 1);
					if (!used[uv] && dist[uv] < dist[yc]) q1.push_back(uv);
				}
				if (y > 0)
				{
					uint8_t uv = (y - 1) * cs + c;
					if (!used[uv] && dist[uv] < dist[yc]) q1.push_back(uv);
				}
				if (y + 1 < ys)
				{
					uint8_t uv = (y + 1) * cs + c;
					if (!used[uv] && dist[uv] < dist[yc]) q1.push_back(uv);
				}
			}
			q0.clear();

			// We randomly take one of the neighbours that is tied to being
			// closest to the origin. By virtue of the time units (see above)
			// this means we prefer to end horizontally, since that gives us
			// the biggest decrease in distance (say from 18 t.u. to 16 t.u.).
			std::random_shuffle(q1.begin(), q1.end());
			std::sort(q1.begin(), q1.end(), [&dist](uint8_t a, uint8_t b){

				return (dist[a] < dist[b]);
			});
			if (q1.size() > 1) q1.resize(1);

			// Assign q0 <= q1 and q1 <= {}.
			q0.swap(q1);
		}
	}

	// We determined the shortest path from the target to the origin.
	std::reverse(record.begin(), record.end());

	// We now convert the shortest path to a PlacementPathing, which is a list
	// of Points with the number of time units attached to them.
	for (uint8_t yc : record)
	{
		uint8_t y = yc / cs;
		uint8_t c = yc % cs;
		uint8_t r = y / 2;

		// We skip the end and the start of the path, since that is known.
		if (y == oldy && c == oldc) continue;
		if (y == newy && c == newc) continue;

		// If _width is not an exact multiple of _boxwidth, we make sure the
		// points are evenly spread out over the entire surface by calculating
		// the relative dx and dy. The point might be in this or in other.
		if (spawn)
		{
			Point point = _topleft;
			int dx = _width  * c / GRIDSIZE;
			int dy = _height * r / GRIDSIZE;
			point.xenon += dx + _boxwidth / 2,
			point.yahoo += dy + (3 + 4 * (y % 2)) * _boxheight / 10;
			box._pathing.emplace_back(point, dist[yc]);
		}
		else if (r / GRIDSIZE == r0 / GRIDSIZE && c / GRIDSIZE == c0 / GRIDSIZE)
		{
			Point point = _topleft;
			int dx = _width  * (c % GRIDSIZE) / GRIDSIZE;
			int dy = _height * (r % GRIDSIZE) / GRIDSIZE;
			point.xenon += dx + _boxwidth / 2,
			point.yahoo += dy + (3 + 4 * (y % 2)) * _boxheight / 10;
			box._pathing.emplace_back(point, dist[yc]);
		}
		else
		{
			Point point = other._topleft;
			int dx = other._width  * (c % GRIDSIZE) / GRIDSIZE;
			int dy = other._height * (r % GRIDSIZE) / GRIDSIZE;
			point.xenon += dx + other._boxwidth / 2,
			point.yahoo += dy + (3 + 4 * (y % 2)) * other._boxheight / 10;
			box._pathing.emplace_back(point, dist[yc]);
		}
	}
	Mixer::get()->queue(Clip::Type::SOLDIERWALK, 0.0f);
}
