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

#include "color.hpp"

struct Pixel;
class SpritePattern;


class Sprite
{
public:
	class Palette
	{
	public:
		Palette(size_t count);
		~Palette();

		Palette(std::vector<Color> colors) :
			Palette(colors.size())
		{
			for (size_t i = 0; i < std::min((size_t) _max, colors.size()); i++)
			{
				_data[i + 1] = colors[i];
			}
		}

	private:
		GLuint _id;
		bool _dirty;
		GLuint _lastShader;
		const uint8_t _max;
		std::vector<Color>  _data;

	public:
		void setColor(size_t index, const Color& color);
		const Color& getColor(size_t index);

		void bind(GLuint shader);
	};

private:
	Sprite(SpritePattern* ptn, std::shared_ptr<Palette> palette);

public:
	Sprite(const std::string& name, std::shared_ptr<Palette> palette = nullptr);
	Sprite(const Sprite&) = delete;
	Sprite(Sprite&&) = delete;
	Sprite& operator=(const Sprite&) = delete;
	Sprite& operator=(Sprite&&) = delete;
	~Sprite();

private:
	const char* _name;
	SpritePattern* const _pattern;

	int _xOffset = 0;
	int _yOffset = 0;
	int _upscale = 1;
	size_t _drawframe = 0;
	size_t _nextframe = 0;
	float _progress = 0;
	float _totalduration = 0;
	std::shared_ptr<Palette> _palette;
	static GLuint _lastShader_static;
	float _shinecolor[3] = {1, 1, 1};

	size_t _start = 0;
	size_t _end = 1;
	std::string _passive;
	std::string _active;
	bool _final = false;
	bool _visible = true;
	bool _border = false;

	float _obscured = 0;
	float _shine = 0;
	float _thetaOffset = 0;

	bool _ninepatch = false;
	int _emptyWidth = 0;
	int _emptyHeight = 0;
	int _innerPatchWidth = 0;
	int _innerPatchHeight = 0;

	bool _isSetAsBackground = false;
	bool _blended = false;
	const SpritePattern* _blendTop = nullptr;
	const SpritePattern* _blendLeft = nullptr;

	void drawStandardQuads(GLuint shader, int x, int y);
	void drawNinePatchQuads(GLuint shader, int x, int y);
	void drawBlendedQuads(GLuint shader, int x, int y, int flags);
	void drawBackgroundQuads(GLuint shader, int x, int y);

public:
	const char* name() const { return _name; }
	void render(GLuint shader, int x, int y, int flags = 0);
	void randomizeAnimationStart();
	void setOriginAtBase();
	void setOriginAtCenter();
	void setOffset(int dx, int dy);
	void setUpscale(int s);
	void setBlendTop(const char* blendname);
	void setBlendLeft(const char* blendname);
	void inheritBlends(std::shared_ptr<Sprite> other);
	void setNinePatch();
	void setAsBackground();
	void trimMargins();
	void setWidth(int w);
	void setHeight(int h);
	void setColor(size_t index, const Color& color);
	void setObscured(float obscured);
	void setShine(float value);
	void setShineColor(const Color& color);
	void setThetaOffset(float offset);
	void setTag(const std::string& tag);
	void setTagActive(const std::string& tag, bool restart = true);
	const Color& getColor(size_t index);
	float getTagDuration(const std::string& tag);
	void setFinal(bool final);
	void setVisible(bool visible);
	void setBorder(bool border);
	bool isFinal() const { return _final; }
	bool isVisible() const { return _visible; }
	bool isBorder() const { return _border; }
	bool isSetAsBackground() const { return _isSetAsBackground; }
	const SpritePattern* backgroundPattern() const { return _pattern; }
	const std::string& getTag() const;
	void update();
	int paletteSize() const;
	int drawscale() const;
	int width() const;
	int height() const;
	int scaledWidth() const;
	int scaledHeight() const;
	int topleftXenon(int originXenon) const;
	int topleftYahoo(int originYahoo) const;
	bool onMask(const Pixel& origin, const Pixel& pixel) const;
};
