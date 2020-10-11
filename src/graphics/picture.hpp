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

struct Texture;
struct Color;
class SpritePattern;


class Picture
{
private:
	Picture(const Texture* t, const std::string& picturename);

public:
	Picture(const std::string& picturename);
	Picture(const Picture&) = delete;
	Picture(Picture&&) = delete;
	Picture& operator=(const Picture&) = delete;
	Picture& operator=(Picture&&) = delete;
	~Picture();

private:
	const bool _missing;
	const std::string _nameOfPicture;
	const Texture* const _texture;

	int _xOffset = 0;
	int _yOffset = 0;
	int _viewWidth = 0;
	int _viewHeight = 0;
	//bool _dirty;
	//GLuint _lastShader;
	static GLuint _lastShader_static;
	float _shinecolor[3] = {1, 1, 1};

	float _grayed = 0;
	float _shine = 0;
	float _thetaOffset = 0;

	bool _isSetAsBackground = false;
	const SpritePattern* _spriteMask = nullptr;

	void drawStandardQuads(GLuint shader, int x, int y);
	void drawBackgroundQuads(GLuint shader, int x, int y);

public:
	bool missing() const { return _missing; }
	const std::string& nameOfPicture() const { return _nameOfPicture; }
	void render(GLuint shader, int x, int y);
	void setAsBackgroundOf(const SpritePattern* pattern);
	void setOffset(int dx, int dy);
	void setWidth(int w);
	void setHeight(int h);
	void setGrayed(float grayed);
	void setShine(float value);
	void setShineColor(const Color& color);
	void setThetaOffset(float offset);
	int width() const;
	int height() const;
	int scaledWidth() const;
	int scaledHeight() const;
	int topleftXenon(int originXenon) const;
	int topleftYahoo(int originYahoo) const;
};
