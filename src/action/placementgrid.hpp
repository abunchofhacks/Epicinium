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

#include "placementbox.hpp"
#include "point.hpp"
#include "descriptor.hpp"

struct PlacementPather;


/*
The PlacementGrid prevents two Figures from occupying the same screenspace on a Surface.
To simplify matters, each Surface is divided into a 5x5 grid and each gridspace can only be
occupied by a single Figure. To complicate matters, tall Figures such as trees and buildings
are not allowed to hide other figures, so they need to reserve two spaces within the grid.
Because we do not want to have to move Figures once they have been placed, "tile figures"
are assigned spaces from a pool that is separate from the pool for other figures.
When created, Figures are assigned a PlacementBox that tells them where to stand.
When the Figure and hence the PlacementBox is destroyed, the space within the grid is released.
*/
class PlacementGrid
{
public:
	PlacementGrid(const Point& topleft, const Point& bottomright);

	PlacementBox place(Descriptor::Type owner);
	PlacementBox place(Descriptor::Type owner, const PlacementBox& oldbox,
		PlacementPather& pather, PlacementGrid& other);

	void spawnPathing(PlacementBox& box, uint8_t buildingindex,
		PlacementPather& pather, PlacementGrid& other);

private:
	const Point _topleft;
	const Point _bottomright;
	const unsigned int _width;
	const unsigned int _height;
	const unsigned int _boxwidth;
	const unsigned int _boxheight;

	std::vector<uint8_t> _possibletileplacements;
	std::vector<uint8_t> _possibleunitplacements;

	std::shared_ptr<PlacementLayer> _tileplacements1;
	std::shared_ptr<PlacementLayer> _tileplacements2;
	std::shared_ptr<PlacementLayer> _unitplacements1;
	std::shared_ptr<PlacementLayer> _unitplacements2;
	std::shared_ptr<PlacementLayer> _airplacements;
	std::shared_ptr<PlacementLayer> _fenceplacements;

	PlacementBox get(const std::shared_ptr<PlacementLayer> layer);
	PlacementBox get(const std::shared_ptr<PlacementLayer> layer,
		uint8_t oldindex, PlacementPather& pather, PlacementGrid& other);
	PlacementBox get(const std::shared_ptr<PlacementLayer> layer,
		uint8_t newindex);


	void addPathing(PlacementBox& box, uint8_t oldindex, bool spawn,
		PlacementPather& pather, PlacementGrid& other);
};
