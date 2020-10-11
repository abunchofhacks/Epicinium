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
#include "camera.hpp"
#include "source.hpp"

#include "settings.hpp"


Camera* Camera::_installed = nullptr;

Camera::Camera(Settings& settings, int width, int height) :
	focus(0, 0),
	offset(0, 0),
	adjust(0, 0),
	WINDOW_W(width),
	WINDOW_H(height),
	SCALE(settings.scale.value())
{}

void Camera::install()
{
	_installed = this;
}

Pixel Camera::convert(const Point& point, const Proximity& z) const
{
	Pixel pixel;
	convertTo(point, z, pixel);
	return pixel;
}

Point Camera::convert(const Pixel& pixel) const
{
	Point point;
	convertTo(pixel, point);
	return point;
}

void Camera::update()
{}
