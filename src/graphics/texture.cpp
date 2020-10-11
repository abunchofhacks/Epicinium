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
#include "texture.hpp"
#include "source.hpp"

#include "libs/SDL2/SDL.h"
#include "libs/SDL2/SDL_image.h"


static std::map<std::string, Texture> _textures = {};

const Texture* Texture::sprite(const std::string& filename)
{
	Texture* result;

	try
	{
		result = &_textures.at(filename);
	}
	catch (const std::out_of_range&)
	{
		SDL_Surface* surface = IMG_Load(filename.c_str());
		if (!surface) throw std::runtime_error("failed to load: " + filename);

		GLuint id;
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		int w = surface->w;
		int h = surface->h;
		glTexImage2D(GL_TEXTURE_2D, 0, surface->format->BytesPerPixel,
			w, h, 0, GL_RED, GL_UNSIGNED_BYTE,
			surface->pixels);
		uint8_t maxindex = 0;
		std::vector<bool> mask(w * h);
		for (int row = 0; row < h; row++)
		{
			for (int col = 0; col < w; col++)
			{
				// Unlike GL textures, SDL surfaces have padding to make their
				// sizes a power of two, which means the pixels are not stored
				// consecutively within surface->pixels; thus 'pitch', not w.
				int offset = row * surface->pitch + col;
				uint8_t index = ((uint8_t*) surface->pixels)[offset];
				if (index > maxindex)
				{
					maxindex = index;
				}
				mask[row * w + col] = (index > 0);
			}
		}
		SDL_FreeSurface(surface);
		_textures.emplace(std::piecewise_construct,
			std::forward_as_tuple(filename),
			std::forward_as_tuple(filename,
				id, maxindex, w, h, std::move(mask)));
		result = &_textures.at(filename);
	}

	return result;
}

const Texture* Texture::picture(const std::string& filename)
{
	Texture* result;

	try
	{
		result = &_textures.at(filename);
	}
	catch (const std::out_of_range&)
	{
		SDL_Surface* surface = IMG_Load(filename.c_str());
		if (!surface) throw std::runtime_error("failed to load: " + filename);

		if (surface->format->format != SDL_PIXELFORMAT_RGBA8888)
		{
			SDL_Surface* newsurface = SDL_ConvertSurfaceFormat(surface,
				SDL_PIXELFORMAT_RGBA8888, 0);
			SDL_FreeSurface(surface);
			surface = newsurface;
		}
		if (!surface) throw std::runtime_error("failed to convert: " + filename);

		GLuint id;
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		int w = surface->w;
		int h = surface->h;
		glTexImage2D(GL_TEXTURE_2D, 0, surface->format->BytesPerPixel,
			w, h, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
			surface->pixels);
		uint8_t maxindex = 0;
		std::vector<bool> mask(w * h, /*memset to */true);
		SDL_FreeSurface(surface);
		_textures.emplace(std::piecewise_construct,
			std::forward_as_tuple(filename),
			std::forward_as_tuple(filename,
				id, maxindex, w, h, std::move(mask)));
		result = &_textures.at(filename);
	}

	return result;
}

Texture::Texture(std::string fn,
	GLuint& id, uint8_t maxPaletteIndex, int width, int height,
	std::vector<bool> mask) :
	_filename(fn),
	_textureID(id),
	_maxPaletteIndex(maxPaletteIndex),
	_width(width),
	_height(height),
	_mask(mask)
{}

Texture::~Texture()
{
	glDeleteTextures(1, &_textureID);
}

std::unique_ptr<Texture> Texture::cutSlice(int x, int y, int w, int h) const
{
	size_t oldsize = (size_t) std::max(0, _width * _height);
	auto pixels = std::vector<uint8_t>(oldsize);
	glBindTexture(GL_TEXTURE_2D, _textureID);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, pixels.data());

	// Collapse the pixels into a corner sized w by h.
	//
	// ------      aaaa--      aaaabb      aaaabb
	// -aaaa-      -aaaa-      bbaaa-      bbcccc       aaaa
	// -bbbb-  =>  -bbbb-  =>  -bbbb-  =>  -bbbb-  =>   bbbb
	// -cccc-      -cccc-      -cccc-      -cccc-       cccc
	// ------      ------      ------      ------
	//
	// Because we do this from front to back, we never overwrite precious data
	// unless it is already copied to the front.
	DEBUG_ASSERT(w > 0 && h > 0);
	DEBUG_ASSERT(w <= _width && h <= _height);
	for (int r = 0; r < h; r++)
	{
		int src = (y + r) * _width + x;
		for (int dst = r * w; dst < r * w + w; dst++)
		{
			DEBUG_ASSERT(dst <= src);
			if (dst >= src) break;
			DEBUG_ASSERT((size_t) src < oldsize);
			pixels[dst] = pixels[src++];
		}
	}
	size_t newsize = (size_t) std::max(0, w * h);
	pixels.resize(newsize);

	GLuint id;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 1,
		w, h, 0, GL_RED, GL_UNSIGNED_BYTE,
		pixels.data());

	// Filename and mask intentionally left empty because they should never be
	// used. SpritePattern::filename() and SpritePattern::onMask() just use
	// the original texture instead.
	return std::unique_ptr<Texture>(new Texture(
		{}, id, _maxPaletteIndex, w, h, {}));
}
