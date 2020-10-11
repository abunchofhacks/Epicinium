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

#include "cell.hpp"


// This class gives range-based-for-loop-access to all the cells within a
// Euclidean ring around a center cell. The min and max are SQUARED EUCLIDEAN
// DISTANCE and are both inclusive. E.g. if the center is at (x, y) and a cell
// is positioned at (x + a, y + b) then it is included if and only if
// min <= (a * a + b * b) <= max, i.e. iff sqrt(min) <= distance <= sqrt(max).

// Some popular rings (denoted min/max):
/*                                                 x     xxxxx   */
/*                 x       x      xxx     xxx     xxx    xxxxx   */
/*         x      xxx     x x     xxx     x x    xxxxx   xxxxx   */
/*                 x       x      xxx     xxx     xxx    xxxxx   */
/*                                                 x     xxxxx   */
/*  1/0   0/0     0/1     1/1     0/2     1/2     0/4     0/8    */

// Distance chart (center at top left):
/*     0   1   4   9   16  25     */
/*     1   2   5   10  17  26     */
/*     4   5   8   13  20  29     */
/*     9   10  13  18  25  34     */
/*     16  17  20  25  32  41     */
/*     25  26  29  34  41  50     */

class Area
{
public:
	Area(Cell centrum, int min, int max) :
		_rows(centrum.rows()),
		_cols(centrum.cols()),
		_centerrow(centrum.pos().row),
		_centercol(centrum.pos().col),
		_min(min),
		_max(max),
		_current(centrum),
		_lastofslice(centrum)
	{}

private:
	const int8_t _rows;
	const int8_t _cols;
	const int8_t _centerrow;
	const int8_t _centercol;
	const int _min;
	const int _max;

	Cell _current;
	Cell _lastofslice;

	// We visit each cell within the bounding box in memory-linear order. For
	// each memory-row, we have either one or two contiguous slices of the
	// circle, depending on whether there is a gap caused by _min.

	static int sliceradius(int dr, int max)
	{
		// The slice radius at row-offset Y is the greatest natural number X for
		// which X * X + Y * Y is smaller or equal to max. This is one less than
		// the least natural number X for which that expression is strictly
		// larger than max.
		int dc;
		for (dc = 1; dc * dc + dr * dr <= max; dc++) {}
		return dc - 1;
	}

	static int innerradius(int dr, int min)
	{
		// The inner radius at row-offset Y is the least natural number X for
		// which X * X + Y * Y is greater or equal to min, which is equal to
		// the least natural number X for which that expression is not strictly
		// smaller than min.
		int dc;
		for (dc = 0; dc * dc + dr * dr < min; dc++) {}
		return dc;
	}

	void firstslice(int currentrow)
	{
		// Jump to the first slice. We might have to do this more than once
		// in case a slice falls entirely outside the borders of the grid
		// or entirely within the gap created by the inner radius, which is
		// marked by having a slice with negative length.

		slice(currentrow - 1, _centercol);

		while (_current.valid() && _lastofslice < _current)
		{
			slice();
		}
	}

	void slice()
	{
		int currentrow = _current.ix() / _cols;
		int lastcol = _current.ix() % _cols;

		return slice(currentrow, lastcol);
	}

	void slice(int currentrow, int lastcol)
	{
		// Determine the left (0) and right (1) column offset ("dc").
		int dc0;
		int dc1;

		// If we are in the right-most slice on this row (that is, the
		// right-hand slice if there is a gap caused by _min or the main slice
		// if there is no such gap in this row), the next slice is in a new row.
		if (lastcol >= _centercol)
		{
			++currentrow;

			int sliceradiusmax = sliceradius(currentrow - _centerrow, _max);
			int sliceradiusmin = innerradius(currentrow - _centerrow, _min);

			// If the inner radius is larger than the slice radius for this
			// slice, the slice is empty so we want to skip it and continue
			// with the first slice on the next row. We mark this by setting
			// the slice length to be negative. We set the current column to be
			// the right-most column so that (lastcol >= _centercol) holds.
			if (sliceradiusmin > sliceradiusmax)
			{
				dc0 = _cols - 1 - _centercol;
				dc1 = dc0 - 1;
			}
			else if (sliceradiusmin > 0)
			{
				dc0 = -sliceradiusmax;
				dc1 = -sliceradiusmin;
			}
			else
			{
				dc0 = -sliceradiusmax;
				dc1 = sliceradiusmax;
			}

			// If the slice exceeds the left grid border, truncate it.
			if (_centercol + dc0 < 0)
			{
				// But if there is a gap in this new row and the entire
				// left-hand slice lies outside the borders, skip it and
				// continue with the right-hand slice.
				if (_centercol + dc1 < 0)
				{
					dc0 = sliceradiusmin;
					dc1 = sliceradiusmax;

					// There is the possibility that the right-hand slice
					// *also* lies entire outside of the borders. We therefore
					// need to repeat this process, and we mark this by setting
					// the slice length to be negative.
					if (_centercol + dc0 >= _cols)
					{
						dc0 = _cols - 1 - _centercol;
						dc1 = dc0 - 1;
					}
				}
				else
				{
					dc0 = -_centercol;
				}
			}

			// If the slice exceeds the right grid border, truncate it.
			if (_centercol + dc1 >= _cols)
			{
				dc1 = _cols - 1 - _centercol;
			}
		}
		// If there is a gap in this row and we are in the left-hand slice,
		// the next slice will be the right-hand slice on the same row.
		else
		{
			dc0 = innerradius(currentrow - _centerrow, _min);
			dc1 = sliceradius(currentrow - _centerrow, _max);

			// If the inner radius is larger than the slice radius for this
			// slice, or if the entire right-hand slice lies outside the grid
			// borders, the slice is empty so we want to skip it and continue
			// with the first slice on the next row. We mark this by setting
			// the slice length to be negative. We set the current column to be
			// the right-most column so that (lastcol >= _centercol) holds.
			if (dc0 > dc1 || _centercol + dc0 >= _cols)
			{
				dc0 = _cols - 1 - _centercol;
				dc1 = dc0 - 1;
			}
			// If the right-hand slice exceeds the grid borders, truncate it.
			else if (_centercol + dc1 >= _cols)
			{
				dc1 = _cols - 1 - _centercol;
			}
		}

		_current.assign(currentrow * _cols + _centercol + dc0);
		_lastofslice.assign(currentrow * _cols + _centercol + dc1);
	}

	void increment()
	{
		if (_current == _lastofslice)
		{
			// Jump to the next slice. We might have to do this more than once
			// in case a slice falls entirely outside the borders of the grid
			// or entirely within the gap created by the inner radius, which is
			// marked by having a slice with negative length.
			do
			{
				slice();
			}
			while (_current.valid() && _lastofslice < _current);
		}
		else ++_current;
	}

public:
	//Area can act as a const_iterator over an area, with Cell as value_type.

	Area& operator++()
	{
		increment();
		return *this;
	}

	Area operator++(int /**/)
	{
		Area iter = *this;
		increment();
		return iter;
	}

	Cell operator*() const
	{
		return _current;
	}

	const Cell* operator->() const
	{
		return &(_current);
	}

	friend bool operator==(const Area& a, const Area& b)
	{
		return (a._current == b._current);
	}

	friend bool operator!=(const Area& a, const Area& b)
	{
		return (a._current != b._current);
	}

	Area begin() const
	{
		Area iter = *this;

		int currentrow = _current.ix() / _cols;
		int firstrow = currentrow - sliceradius(0, _max);
		if (firstrow < 0) firstrow = 0;

		iter.firstslice(firstrow);

		return iter;
	}

	Area end() const
	{
		Area iter = *this;

		int currentrow = _current.ix() / _cols;
		int r1 = currentrow + sliceradius(0, _max) + 1;
		if (r1 > _rows) r1 = _rows;

		iter.firstslice(r1);

		return iter;
	}
};

