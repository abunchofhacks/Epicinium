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

#include "libs/GLEW/glew.h"

#define INTERPOLATE(A, B, RATIO) ((1 - (RATIO)) * (A) + (RATIO) * (B))

struct Color
{
	GLubyte r, g, b, a;

	constexpr Color(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha) :
		r(red), g(green), b(blue), a(alpha)
	{}
	constexpr Color(GLubyte red, GLubyte green, GLubyte blue) :
		Color(red, green, blue, 255)
	{}
	constexpr Color() :
		Color(0, 0, 0, 0)
	{}

	bool operator==(const Color& other) const
	{
		return (r == other.r && g == other.g && b == other.b && a == other.a);
	}

	bool operator!=(const Color& other) const
	{
		return !(*this == other);
	}

	explicit operator bool() const
	{
		return (r || g || b || a);
	}

	static Color mix(const Color& a, const Color& b, int ratio) = delete;

	static constexpr Color mix(const Color& a, const Color& b, float ratio)
	{
		return Color(
			(GLubyte) INTERPOLATE(a.r, b.r, ratio),
			(GLubyte) INTERPOLATE(a.g, b.g, ratio),
			(GLubyte) INTERPOLATE(a.b, b.b, ratio),
			(GLubyte) INTERPOLATE(a.a, b.a, ratio)
		);
	}

	static constexpr Color blend(const Color& a, const Color& b)
	{
		return blend(a, b, 1.0f);
	}

	static Color blend(const Color& a, const Color& b, int ratio) = delete;

	static constexpr Color blend(const Color& a, const Color& b, float ratio)
	{
		return Color(
			(GLubyte) INTERPOLATE(a.r, b.r, b.a * (1.0f / 255) * ratio),
			(GLubyte) INTERPOLATE(a.g, b.g, b.a * (1.0f / 255) * ratio),
			(GLubyte) INTERPOLATE(a.b, b.b, b.a * (1.0f / 255) * ratio),
			a.a
		);
	}

	static constexpr Color alpha(const Color& a, int value)
	{
		return Color(a.r, a.g, a.b, (GLubyte) value);
	}

	static Color alpha(const Color& a, float ratio) = delete;

	static constexpr Color undefined()
	{
		// Undefined is the only color that evaluates to false.
		return Color();
	}

	static constexpr Color transparent()
	{
		// Transparent pink to differentiate from undefined.
		return Color(255, 0, 255, 0);
	}

	static constexpr Color broken()
	{
		// Traditional bright pink.
		return Color(255, 0, 255);
	}

	std::string toString() const;
	static Color parse(std::string str);
};
