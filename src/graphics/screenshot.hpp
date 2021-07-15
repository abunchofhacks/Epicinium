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


class Screenshot
{
public:
	std::string _tag;
	GLuint _framebufferID;
	GLuint _textureID;
	const int _width;
	const int _height;

public:
	Screenshot(int width, int height, const std::string& tag = "",
		bool renderable = true);
	~Screenshot();
	Screenshot(const Screenshot&) = delete;
	Screenshot(Screenshot&&) = delete;
	Screenshot& operator=(const Screenshot&) = delete;
	Screenshot& operator=(Screenshot&&) = delete;

	void setAsRenderTarget();
	void finishRendering();

	const std::string& tag() const { return _tag; }
	GLuint textureID() const { return _textureID; }
	int width() const { return _width; }
	int height() const { return _height; }

	std::vector<uint8_t> writeToBuffer();
};
