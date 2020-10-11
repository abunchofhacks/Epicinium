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
#include "square.hpp"
#include "border.hpp"

enum class Season : uint8_t;
enum class Daytime : uint8_t;
enum class Phase : uint8_t;
struct Point;
struct Change;
struct AnimationGroup;


class Level
{
private:
	// Cells is a std::vector<Square> that contains a hidden value at the end
	// that only Level itself can access, which isn't revealed when iterating.
	class Cells : private std::vector<Square>
	{
	private:
		friend class Level;

	public:
		typename std::vector<Square>::const_iterator begin() const
		{
			return std::vector<Square>::begin();
		}

		typename std::vector<Square>::const_iterator end() const
		{
			return std::vector<Square>::end() - 1;
		}

		typename std::vector<Square>::iterator begin()
		{
			return std::vector<Square>::begin();
		}

		typename std::vector<Square>::iterator end()
		{
			return std::vector<Square>::end() - 1;
		}
	};

public:
	Level(const Bible& bible, const Skinner& skinner);

	Level(const Level&) = delete;
	Level(Level&&) = delete;
	Level& operator=(const Level&) = delete;
	Level& operator=(Level&&) = delete;
	~Level() = default;

private:
	const Bible& _bible;
	const Skinner& _skinner;

	int _cols;
	int _rows;

	int _year;
	Season _season;
	Daytime _daytime;
	Phase _phase;

	Cells _cells;

	std::vector<Border> _borders;

	void resize(int cols, int rows);

	Square& at(int r, int c)
	{
		return _cells[checkedindex(r, c)];
	}

	const Square& at(int r, int c) const
	{
		return _cells[checkedindex(r, c)];
	}

	Square& at(const Position& position)
	{
		return at(position.row, position.col);
	}

	const Square& at(const Position& position) const
	{
		return at(position.row, position.col);
	}

public:
	int rows() const { return _rows; }
	int cols() const { return _cols; }

	void clear(int cols, int rows);

private:
	void enactInSubject(const Change& change, std::shared_ptr<AnimationGroup> group);

	int checkedindex(int r, int c) const
	{
		return (r >= 0 && c >= 0 && r < _rows && c < _cols)
			? (r * _cols + c)
			: (_rows * _cols);
	}

public:
	void generateForests();
	void generateBorders();

private:
	int borderDistance(int r, int c);
	void populateBorder(Border& border, int thickness, int rowlen,
			const std::vector<TileType>& types,
		const std::vector<TreeType>& treetypes);

public:
	Point centerPoint();
	Point topleftPoint();
	Point bottomrightPoint();

	void update();
	void setLight(float light, float duration, float delay = 0);
	void setBorderLight(float light, float duration, float delay = 0);
	void setBorderSpotlight(float light, float duration, float delay = 0);
	void enact(const Change& change, std::shared_ptr<AnimationGroup> group);

	Cell index(const Position& pos) const
	{
		return Cell::create(_rows, _cols, checkedindex(pos.row, pos.col));
	}

	size_t size() const { return _cells.size(); }

	Square* cell(const Position& position)
	{
		return &at(position);
	}

	const Square* cell(const Position& position) const
	{
		return &at(position);
	}

	typename std::vector<Square>::const_iterator begin() const
	{
		return _cells.begin();
	}

	typename std::vector<Square>::const_iterator end() const
	{
		return _cells.end();
	}

	typename std::vector<Square>::iterator begin()
	{
		return _cells.begin();
	}

	typename std::vector<Square>::iterator end()
	{
		return _cells.end();
	}

	Area area(Cell centrum, int min, int max) const
	{
		return Area(centrum, min, max);
	}

	Area area(const Position& centrum, int min, int max) const
	{
		return Area(index(centrum), min, max);
	}
};
