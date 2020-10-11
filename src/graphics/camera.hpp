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

	Camera(Settings& settings, int width, int height);

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
		pixel.xenon = FOCUS_XENON + offset.xenon + adjust.xenon + SCALE * dx;
		pixel.yahoo = FOCUS_YAHOO + offset.yahoo + adjust.yahoo + SCALE * dy;
		pixel.proximity = z;
	}

	Point convert(const Pixel& pixel) const;
	void convertTo(const Pixel& pixel, Point& point) const
	{
		int dx = pixel.xenon - FOCUS_XENON - offset.xenon - adjust.xenon;
		int dy = pixel.yahoo - FOCUS_YAHOO - offset.yahoo - adjust.yahoo;
		point.xenon = focus.xenon + dx / SCALE;
		point.yahoo = focus.yahoo + dy / SCALE;
	}

	bool isOnScreen(const Point& point) const
	{
		Pixel pixel;
		convertTo(point, pixel);
		return isOnScreen(pixel);
	}

	bool isOnScreen(const Pixel& pixel) const
	{
		return (   pixel.xenon >= 0 && pixel.xenon <= WINDOW_W
				&& pixel.yahoo >= 0 && pixel.yahoo <= WINDOW_H);
	}

	void adjustFocus(const Pixel& pixel)
	{
		adjust.xenon = pixel.xenon - FOCUS_XENON;
		adjust.yahoo = pixel.yahoo - FOCUS_YAHOO;
	}

	void update();

private:
	friend class CameraFocus;
	Point focus;
	Pixel offset;
	Pixel adjust;

public:
	const int WINDOW_W;
	const int WINDOW_H;
	const int SCALE;

private:
	const int FOCUS_XENON = WINDOW_W / 2;
	const int FOCUS_YAHOO = WINDOW_H / 2;
};
