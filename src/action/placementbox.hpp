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

#include "point.hpp"
#include "randomizer.hpp"


class PlacementLayer : public Randomizer<uint8_t>
{
	// Just a rename.
};

class PlacementPathing : public std::vector<std::pair<Point, uint8_t>>
{
	// Just a rename.
};

/*
When a PlacementBox is assigned to a Figure, that Figure is allowed to be placed somewhere
within the box. No other Figures will be assigned the same space within the PlacementGrid.
When the Figure and hence the PlacementBox is destroyed, the space within the grid is released.
*/
class PlacementBox
{
public:
	PlacementBox(const std::shared_ptr<PlacementLayer> owner,
			uint8_t index, const Point& topleft, const Point& bottomright) :
		_owner(owner),
		_index(index),
		_topleft(topleft),
		_bottomright(bottomright),
		_pathing({}),
		_raised(false),
		_lowered(false),
		_entrenched(false),
		_enfenced(false)
	{}

	PlacementBox(const PlacementBox&) = delete;

	PlacementBox(PlacementBox&& that) :
		_owner(that._owner),
		_index(that._index),
		_topleft(that._topleft),
		_bottomright(that._bottomright),
		_pathing(std::move(that._pathing)),
		_raised(that._raised),
		_lowered(that._lowered),
		_entrenched(that._entrenched),
		_enfenced(that._enfenced)
	{
		that._owner.reset();
	}

/*
	PlacementBox& operator=(const PlacementBox&) = delete;
	PlacementBox& operator=(PlacementBox&&) = delete;
*/
	PlacementBox& operator=(PlacementBox that)
	{
		this->swap(that);
		return *this;
	}

	void swap(PlacementBox& that)
	{
		std::swap(_owner, that._owner);
		std::swap(_index, that._index);
		std::swap(_topleft, that._topleft);
		std::swap(_bottomright, that._bottomright);
		std::swap(_pathing, that._pathing);
		std::swap(_raised, that._raised);
		std::swap(_lowered, that._lowered);
		std::swap(_entrenched, that._entrenched);
		std::swap(_enfenced, that._enfenced);
	}

	~PlacementBox()
	{
		release();
	}

	void release()
	{
		if (auto owner = _owner.lock())
		{
			owner->push(_index);
		}
		_owner.reset();
	}

private:
	friend class PlacementGrid;

	std::weak_ptr<PlacementLayer> _owner;
	uint8_t _index;

	Point _topleft;
	Point _bottomright;

	PlacementPathing _pathing;

	bool _raised;
	bool _lowered;
	bool _entrenched;
	bool _enfenced;

public:
	Point random();

	// This puts the point slightly higher.
	void raise()
	{
		_raised = true;
	}

	// This puts the point slightly lower.
	void lower()
	{
		_lowered = true;
	}

	// This disables the height randomization.
	void entrench()
	{
		_entrenched = true;

		centralize(15, 0, 15, 0);
	}

	// This puts the point at the bottom of the box.
	void enfence()
	{
		_enfenced = true;
	}

	bool entrenched()
	{
		return _entrenched;
	}

	// This method pushes the corners of the placementbox inwards by a percentage,
	// towards the center of the placementgrid that this placement box belongs to.
	// A negative percents pushes the corners outwards away from the center.
	// PercentE indicates how much Points east of the center should be pushed to the left.
	void centralize(int8_t percentE, int8_t percentS, int8_t percentW, int8_t percentN);

	// The PlacementGrid can determine a pathing around other placed figures.
	PlacementPathing& pathing()
	{
		return _pathing;
	}

	// TODO it is a bit ugly that Tile and Figure have to use this "box index".
	uint8_t index()
	{
		return _index;
	}
};
