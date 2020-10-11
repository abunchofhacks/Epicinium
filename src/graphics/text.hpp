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

struct TextStyle;


class Text
{
public:
	Text(const std::string& text, const TextStyle& style);
	Text(const Text&) = delete;
	Text(Text&&) = delete;
	Text& operator=(const Text&) = delete;
	Text& operator=(Text&&) = delete;
	~Text();

private:
	const std::string _text;
	float _alpha = 1;

	int _width = 0;
	int _height = 0;
	float _s = 0;
	float _t = 0;
	GLuint _textureID = 0;

	static GLuint _lastShader_static;

public:
	void render(GLuint shader, int x, int y);
	const std::string& text() const { return _text; }
	int width() const { return _width; }
	int height() const { return _height; }

	void setAlpha(float alpha) { _alpha = alpha; }

	static int fontH(int fontsize);
	static int fontH(const TextStyle& style);

	static int calculateWidth(const TextStyle& style, const std::string& text);
};
