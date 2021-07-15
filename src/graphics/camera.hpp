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
#include "pixel.hpp"


class Settings;

class Camera
{
private:
	static Camera* _installed;

public:
	static Camera* get() { return _installed; }

	Camera(int width, int height, int scale);

	Camera(const Camera&) = delete;
	Camera(Camera&&) = delete;
	Camera& operator=(const Camera&) = delete;
	Camera& operator=(Camera&&) = delete;
	~Camera() = default;

	void install();

	Pixel convert(const Point& point) const
	{
		return convert(point, Proximity());
	}
	Pixel convert(const Point& point, const Proximity& z) const;
	void convertTo(const Point& point, Pixel& pixel) const
	{
		return convertTo(point, Proximity(), pixel);
	}
	void convertTo(const Point& point, const Proximity& z, Pixel& pixel) const
	{
		int dx = point.xenon - focus.xenon;
		int dy = point.yahoo - focus.yahoo;
		pixel.xenon = _focusXenon + offset.xenon + adjust.xenon + _scale * dx;
		pixel.yahoo = _focusYahoo + offset.yahoo + adjust.yahoo + _scale * dy;
		pixel.proximity = z;
	}

	Point convert(const Pixel& pixel) const;
	void convertTo(const Pixel& pixel, Point& point) const
	{
		int dx = pixel.xenon - _focusXenon - offset.xenon - adjust.xenon;
		int dy = pixel.yahoo - _focusYahoo - offset.yahoo - adjust.yahoo;
		point.xenon = focus.xenon + dx / _scale;
		point.yahoo = focus.yahoo + dy / _scale;
	}

	bool isOnScreen(const Point& point) const
	{
		Pixel pixel;
		convertTo(point, pixel);
		return isOnScreen(pixel);
	}

	bool isOnScreen(const Pixel& pixel) const
	{
		return (   pixel.xenon >= 0 && pixel.xenon <= _width
				&& pixel.yahoo >= 0 && pixel.yahoo <= _height);
	}

	void adjustFocus(const Pixel& pixel)
	{
		adjust.xenon = pixel.xenon - _focusXenon;
		adjust.yahoo = pixel.yahoo - _focusYahoo;
	}

	void update();

private:
	friend class CameraFocus;
	Point focus;
	Pixel offset;
	Pixel adjust;

	int _width;
	int _height;
	int _focusXenon;
	int _focusYahoo;
	int _scale;

public:
	int width() const { return _width; }
	int height() const { return _height; }
	int scale() const { return _scale; }

	void changeViewport(int width, int height);
	void changeScale(int scale);
};
