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


struct Texture
{
	Texture(std::string fn, GLuint& id,
		uint8_t maxPaletteIndex, int width, int height,
		std::vector<bool> mask);
	~Texture();
	Texture(const Texture&) = delete;
	Texture(Texture&&) = delete;
	Texture& operator=(const Texture&) = delete;
	Texture& operator=(Texture&&) = delete;

	const std::string _filename;
	const GLuint _textureID;
	const uint8_t _maxPaletteIndex;
	const int _width;
	const int _height;
	const std::vector<bool> _mask;

	static const Texture* sprite(const std::string& filename);
	static const Texture* picture(const std::string& filename);

	std::unique_ptr<Texture> cutSlice(int x, int y, int w, int h) const;
};
