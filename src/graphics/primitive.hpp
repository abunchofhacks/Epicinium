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

struct Color;


class Primitive
{
public:
	enum class Type
	{
		RECTANGLE_FILLED,
		RECTANGLE_OUTLINE,
		ELLIPSE_FILLED,
		ELLIPSE_OUTLINE,
	};

	Primitive(const Type& type, const Color& color, int width, int height);
	Primitive(const Primitive&) = delete;
	Primitive(Primitive&&) = delete;
	Primitive& operator=(const Primitive&) = delete;
	Primitive& operator=(Primitive&&) = delete;
	~Primitive();

private:
	const Type _type;
	const int _width = 0;
	const int _height = 0;
	float _s = 0;
	float _t = 0;
	float _obscured = 0;
	float _shine = 0;
	float _shinecolor[3] = {1, 1, 1};
	float _thetaOffset = 0;
	GLuint _textureID = 0;

	static GLuint _lastShader_static;

public:
	void render(GLuint shader, int x, int y);
	int width() const { return _width; }
	int height() const { return _height; }

	void setObscured(float obscured);
	void setShine(float value);
	void setShineColor(const Color& color);
	void setThetaOffset(float offset);
};
