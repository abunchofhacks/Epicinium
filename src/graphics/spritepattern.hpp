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

#include <map>

#include "texture.hpp"


struct Slice
{
	int xOnTexture;
	int yOnTexture;
	float left;
	float center;
	float right;
	float top;
	float middle;
	float bottom;
	int w;
	int h;
	int x;
	int y;
	float duration;
};

struct Tag
{
	size_t start;
	size_t end;

	Tag(size_t s, size_t e) : start(s), end(e) {}
	Tag() : start(0), end(0) {}
};

class SpritePattern
{
private:
	static std::string _spritesfolder;

public:
	static void setResourceRoot(const std::string& root);

	SpritePattern(std::string sn, const Texture* tx);
	~SpritePattern();
	SpritePattern(const SpritePattern&) = delete;
	SpritePattern(SpritePattern&&) = delete;
	SpritePattern& operator=(const SpritePattern&) = delete;
	SpritePattern& operator=(SpritePattern&&) = delete;

private:
	static std::map<std::string, SpritePattern> _spritepatterns;

	const std::string _name;
	const Texture* const _texture;
	int _width;
	int _height;
	std::vector<Slice> _slices; // (married)
	std::vector<std::unique_ptr<Texture>> _slicetextures; // (married or empty)
	std::map<std::string, Tag> _tags;

	bool _trimmed = false;
	bool _ninepatch = false;
	int _leftMarginWidth = 0;
	int _rightMarginWidth = 0;
	int _topMarginHeight = 0;
	int _bottomMarginHeight = 0;
	int _innerPatchWidth = 0;
	int _innerPatchHeight = 0;
	int _leftPatchWidth = 0;
	int _rightPatchWidth = 0;
	int _topPatchHeight = 0;
	int _bottomPatchHeight = 0;

	void addSlice(int tx, int ty, int w, int h, int sx, int sy, float duration);
	void addTag(const std::string& tag, size_t start, size_t end);

	static void load(const std::string& name,
		const std::string& imagefilename, const Json::Value& json);

public:
	static void preloadFromIndex();

	static SpritePattern* get(const std::string& name);

	void trim();
	void cutNinePatch();
	void cutSlicesIntoSeparateTextures();

	const char* name() const { return _name.c_str(); }
	const char* filename() const { return _texture->_filename.c_str(); }
	uint8_t maxPaletteIndex() const { return _texture->_maxPaletteIndex; }
	int width() const
	{
		return _width;
	}
	int height() const
	{
		return _height;
	}
	int leftMarginWidth() const { return _leftMarginWidth; }
	int rightMarginWidth() const { return _rightMarginWidth; }
	int topMarginHeight() const { return _topMarginHeight; }
	int bottomMarginHeight() const { return _bottomMarginHeight; }
	int innerPatchWidth() const { return _innerPatchWidth; }
	int innerPatchHeight() const { return _innerPatchHeight; }
	int leftPatchWidth() const { return _leftPatchWidth; }
	int rightPatchWidth() const { return _rightPatchWidth; }
	int topPatchHeight() const { return _topPatchHeight; }
	int bottomPatchHeight() const { return _bottomPatchHeight; }
	bool onMask(size_t index, size_t x, size_t y) const;

	size_t slices() { return _slices.size(); }
	const Slice& slice(size_t index) { return _slices[index]; }
	const Tag& tag(const std::string& name) { return _tags[name]; }
	float getTagDuration(const std::string& tag);

	GLuint texture(size_t index = 0) const
	{
		if (index < _slicetextures.size())
		{
			return _slicetextures[index]->_textureID;
		}
		else return _texture->_textureID;
	}
};
