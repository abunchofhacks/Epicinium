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

#include "animator.hpp"
#include "position.hpp"

enum class TileType : uint8_t;
enum class TreeType : uint8_t;
class PlacementGrid;
class Surface;
class Figure;
class Bible;
class Skinner;


class Border : private Animator
{
public:
	Border(const Position& position,
		const Bible& bible, const Skinner& skinner,
		int distance);
	Border(const Border&) = delete;
	Border(Border&& that);
	~Border();

	Border& operator=(const Border&) = delete;
	Border& operator=(Border&&) = delete;

private:
	const Position _position;
	const Bible& _bible;
	const Skinner& _skinner;

	uint8_t _distance;
	TileType _type;
	TileType _east;
	TileType _south;
	TileType _west;
	TileType _north;
	TreeType _treetype;

	// Note that _placementgrid must be destroyed after surfaces and figures.
	std::unique_ptr<PlacementGrid> _placementgrid;
	std::vector<std::shared_ptr<Surface>> _surfaces; // (unique ownership)
	std::vector<std::shared_ptr<Figure>> _figures; // (unique ownership)
	size_t _nfigures;

	std::shared_ptr<Surface> newSurface();
	std::shared_ptr<Figure> newFigure();

	void cleanup();

public:
	void set(const TileType& tiletype,
		const TileType& e, const TileType& s,
		const TileType& w, const TileType& n,
		const TreeType& treetype);

	void setAsBackground();
	void populate();

	void update();

	void setLight(float light, float duration, float delay);
	void setSpotlight(float light, float duration, float delay);

	const Position& position() const { return _position; }
	const Skinner& skinner() const { return _skinner; }
};
