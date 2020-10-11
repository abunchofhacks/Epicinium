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
#include "spritepattern.hpp"
#include "source.hpp"


std::string SpritePattern::_spritesfolder = "sprites/";

void SpritePattern::setResourceRoot(const std::string& root)
{
	if (root.empty())
	{
		_spritesfolder = "sprites/";
	}
	else if (root.back() == '/')
	{
		_spritesfolder = root + "sprites/";
	}
	else
	{
		_spritesfolder = root + "/sprites/";
	}
}

std::map<std::string, SpritePattern> SpritePattern::_spritepatterns = {};

void SpritePattern::preloadFromIndex()
{
	std::string indexfname = _spritesfolder + "index.list";
	std::ifstream indexfile(indexfname);
	if (!indexfile.is_open())
	{
		LOGF << "Could not open " << indexfname;
		throw std::runtime_error("Could not open sprites/index.list");
	}

	Json::Reader reader;
	Json::Value json;
	std::string path;
	while (std::getline(indexfile, path))
	{
		if (path.length() == 0) continue;

		// The index file might have changed from LF to CRLF somehow.
		if (path.back() == '\r')
		{
			path.pop_back();
		}

		// Extract the sprite name.
		if (path.length() <= 8 + 5
			|| path.compare(0, 8, "sprites/") != 0
			|| path.compare(path.size() - 5, 5, ".json") != 0)
		{
			LOGE << "Ignoring invalid sprite data filename " << path;
			DEBUG_ASSERT(false);
			continue;
		}
		std::string name = path.substr(8, path.size() - 5 - 8);

		// The paths in the indexfile are relative, so we need to adjust them
		// to the spritesfolder.
		path = _spritesfolder + name + ".json";

		if (!System::isFile(path))
		{
			LOGF << "Failed to load sprite " << name
				<< ": file '" << path << "' does not exist";
			throw std::runtime_error("Failed to load sprite " + name
				+ ": file does not exist");
		}

		std::ifstream file(path);
		if (!file.is_open() || !reader.parse(file, json))
		{
			LOGF << "Failed to load sprite " << name
				<< ": " << reader.getFormattedErrorMessages();
			throw std::runtime_error("Failed to load sprite " + name
				+ ": " + reader.getFormattedErrorMessages());
		}

		if (!json["meta"].isObject())
		{
			LOGF << "Sprite '" << name << "' has no metadata";
			throw std::runtime_error("sprite '" + std::string(name)
				+ "' has no metadata");
		}
		if (!json["meta"]["image"].isString())
		{
			LOGF << "Sprite '" << name << "' has no file attached";
			throw std::runtime_error("sprite '" + std::string(name)
				+ "' has no file attached");
		}
		std::string imagefilename = json["meta"]["image"].asString();
		// Since Aseprite version 1.2.10, "image" is a filename not a full path.
		// Also the filename should be relative, so we need to adjust it
		// to the spritesfolder.
		if (imagefilename.find_first_of('/') == std::string::npos)
		{
			imagefilename = path.substr(0, path.find_last_of('/'))
				+ "/" + imagefilename;
		}
		else
		{
			imagefilename = path.substr(0, path.find_last_of('/'))
				+ imagefilename.substr(imagefilename.find_last_of('/'));
		}

		load(name, imagefilename, json);
	}
}

void SpritePattern::load(const std::string& spritename,
	const std::string& imagefilename, const Json::Value& json)
{
	const Texture* texture = Texture::sprite(imagefilename);
	_spritepatterns.emplace(std::piecewise_construct,
		std::forward_as_tuple(spritename),
		std::forward_as_tuple(spritename, texture));
	SpritePattern& pattern = _spritepatterns.at(spritename);

	if (!json["frames"].isArray() || json["frames"].size() == 0)
	{
		LOGF << "Sprite '" << spritename << "' has no frames";
		throw std::runtime_error("sprite '" + std::string(spritename)
			+ "' has no frames");
	}
	auto& clips = json["frames"];
	if (clips[0]["frame"].isNull())
	{
		LOGF << "Sprite '" << spritename << "' has invalid first frame";
		throw std::runtime_error("Sprite '" + std::string(spritename)
			+ "' has invalid first frame");
	}

	{
		auto& source = json["frames"][0]["sourceSize"];
		if (source.isNull() || source["w"].isNull() || source["h"].isNull())
		{
			LOGF << "Sprite '" << spritename << "' has invalid sourceSize";
			throw std::runtime_error("sprite '" + std::string(spritename)
				+ "' has invalid sourceSize");
		}
		pattern._width = source["w"].asInt();
		pattern._height = source["h"].asInt();
	}

	for (auto& clip : clips)
	{
		auto& frame = clip["frame"];
		auto& source = clip["spriteSourceSize"];
		if (frame.isNull() || frame["x"].isNull() || frame["y"].isNull()
			|| frame["w"].isNull() || frame["h"].isNull()
			|| source["x"].isNull() || source["y"].isNull()
			|| clip["duration"].isNull())
		{
			LOGF << "Sprite '" << spritename << "' has invalid frame";
			throw std::runtime_error("sprite '" + std::string(spritename)
				+ "' has invalid frame");
		}
		pattern.addSlice(frame["x"].asInt(), frame["y"].asInt(),
			frame["w"].asInt(), frame["h"].asInt(),
			source["x"].asInt(), source["y"].asInt(),
			0.001f * clip["duration"].asInt());
	}

	auto& tags = json["meta"]["frameTags"];
	auto& layers = json["meta"]["layers"];
	if (tags.isArray() && tags.size() > 0)
	{
		for (auto& tag : tags)
		{
			pattern.addTag(tag["name"].asString(),
				tag["from"].asInt(), tag["to"].asInt() + 1);
		}
	}
	else if (layers.isArray() && layers.size() > 0)
	{
		for (size_t i = 0; i < layers.size(); i++)
		{
			auto& layer = layers[(int) i];
			pattern.addTag(layer["name"].asString(), i, i + 1);
		}
	}
}

SpritePattern::SpritePattern(std::string nm, const Texture* tex) :
	_name(nm),
	_texture(tex)
{}

SpritePattern::~SpritePattern()
{}

void SpritePattern::addSlice(int x, int y, int w, int h, int sx, int sy,
	float duration)
{
	_slices.push_back({
		x,
		y,
		(1.0f * x           ) / _texture->_width,
		(1.0f * x + 0.5f * w) / _texture->_width,
		(1.0f * x +        w) / _texture->_width,
		(1.0f * y           ) / _texture->_height,
		(1.0f * y + 0.5f * h) / _texture->_height,
		(1.0f * y +        h) / _texture->_height,
		w,
		h,
		sx,
		sy,
		duration
	});
}

void SpritePattern::addTag(const std::string& tag, size_t start, size_t end)
{
	if (end > _slices.size())
	{
		LOGF << "invalid tag with end " << std::to_string(end)
			<< " for SpritePattern with " << std::to_string(_slices.size())
			<< " slices";
		throw std::runtime_error("invalid tag with end " + std::to_string(end)
			+ " for SpritePattern with " + std::to_string(_slices.size())
			+ " slices");
	}

	_tags[tag] = {start, end};
}

SpritePattern* SpritePattern::get(const std::string& name)
{
	auto found = _spritepatterns.find(name);
	if (found == _spritepatterns.end())
	{
		LOGF << "Missing sprite '" << name << "'";
	}
	return &_spritepatterns.at(name);
}

float SpritePattern::getTagDuration(const std::string& tag)
{
	if (!_tags.count(tag)) return 0.0f;
	float totalduration = 0.0f;
	for (size_t i = _tags[tag].start; i < _tags[tag].end; i++)
	{
		totalduration += _slices[i].duration;
	}
	return totalduration;
}

void SpritePattern::trim()
{
	if (_trimmed) return;
	_trimmed = true;

	int trimmedWidth = _slices[0].w;
	int trimmedHeight = _slices[0].h;
	_leftMarginWidth = _slices[0].x;
	_topMarginHeight = _slices[0].y;
	_rightMarginWidth = _width - trimmedWidth - _leftMarginWidth;
	_bottomMarginHeight = _height - trimmedHeight - _topMarginHeight;
	_width = trimmedWidth;
	_height = trimmedHeight;
}

void SpritePattern::cutNinePatch()
{
	if (_ninepatch) return;
	_ninepatch = true;

	int cc[6] = {0};
	int rr[6] = {0};

	cc[0] = 0;
	cc[5] = _width;
	rr[0] = 0;
	rr[5] = _height;

	int r, c;
	r = 0;
	for (c = 0; c < _width; c++)
	{
		if (onMask(0, c, r))
		{
			cc[2] = c;
			break;
		}
	}
	for ( c++ ; c < _width; c++)
	{
		if (!onMask(0, c, r))
		{
			cc[3] = c;
			break;
		}
	}
	c = 0;
	for (r = 0; r < _height; r++)
	{
		if (onMask(0, c, r))
		{
			rr[2] = r;
			break;
		}
	}
	for ( r++ ; r < _height; r++)
	{
		if (!onMask(0, c, r))
		{
			rr[3] = r;
			break;
		}
	}

	DEBUG_ASSERT(cc[2] < cc[3] && rr[2] < rr[3]);

	cc[1] = cc[0] + 1;
	cc[4] = cc[3];
	rr[1] = rr[0] + 1;
	rr[4] = rr[3];

	for (c = cc[0] + 1; c < cc[2]; c++)
	{
		bool content = false;
		for (r = rr[1]; r < rr[4]; r++)
		{
			if (onMask(0, c, r))
			{
				content = true;
				break;
			}
		}
		if (content) break;

		cc[1] = c + 1;
	}

	for (c = cc[3]; c < cc[5] - 1; c++)
	{
		bool content = false;
		for (r = rr[1]; r < rr[4]; r++)
		{
			if (onMask(0, c, r))
			{
				content = true;
				break;
			}
		}
		if (!content) break;

		cc[4] = c + 1;
	}

	for (r = rr[0] + 1; r < rr[2]; r++)
	{
		bool content = false;
		for (c = cc[1]; c < cc[4]; c++)
		{
			if (onMask(0, c, r))
			{
				content = true;
				break;
			}
		}
		if (content) break;

		rr[1] = r + 1;
	}

	for (r = rr[3]; r < rr[5] - 1; r++)
	{
		bool content = false;
		for (c = cc[1]; c < cc[4]; c++)
		{
			if (onMask(0, c, r))
			{
				content = true;
				break;
			}
		}
		if (!content) break;

		rr[4] = r + 1;
	}

	_leftMarginWidth    = cc[1] - cc[0];
	_leftPatchWidth     = cc[2] - cc[1];
	_innerPatchWidth    = cc[3] - cc[2];
	_rightPatchWidth    = cc[4] - cc[3];
	_rightMarginWidth   = cc[5] - cc[4];

	_topMarginHeight    = rr[1] - rr[0];
	_topPatchHeight     = rr[2] - rr[1];
	_innerPatchHeight   = rr[3] - rr[2];
	_bottomPatchHeight  = rr[4] - rr[3];
	_bottomMarginHeight = rr[5] - rr[4];
}

void SpritePattern::cutSlicesIntoSeparateTextures()
{
	if (_slices.size() <= 1) return;

	if (!_slicetextures.empty()) return;

	_slicetextures.reserve(_slices.size());
	for (const Slice& slice : _slices)
	{
		auto sliced = _texture->cutSlice(slice.xOnTexture, slice.yOnTexture,
			slice.w, slice.h);
		_slicetextures.emplace_back(std::move(sliced));
	}
}

bool SpritePattern::onMask(size_t index, size_t x, size_t y) const
{
	const Slice& slice = _slices[index];

	// If Sprite does not perceive this SpritePattern as "trimmed", then it
	// thinks that there is whitespace around the ink. The x and y will be the
	// top left of the entire slice, including this virtual whitespace.
	int tx = x - (!_trimmed) * slice.x;
	int ty = y - (!_trimmed) * slice.y;

	// If the on-texture x and y are outside of the on-texture dimensions of the
	// slice, it is not on the mask. This is not (just) an optimization; the
	// texture might be tightly packed with other delicious slices or patterns,
	// but virtually there is a single slice in an endless ocean of whitespace.
	if (tx < 0 || ty < 0 || tx >= slice.w || ty >= slice.h)
	{
		return false;
	}

	int col = slice.xOnTexture + tx;
	int row = slice.yOnTexture + ty;

	return _texture->_mask[row * _texture->_width + col];
}
