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
#include "text.hpp"
#include "source.hpp"

#include "libs/SDL2/SDL.h"
#include "libs/SDL2/SDL_ttf.h"

#include "color.hpp"
#include "textstyle.hpp"
#include "paint.hpp"
#include "graphics.hpp"


GLuint Text::_lastShader_static = 0;

static std::string hexify(const std::string& text);

Text::Text(const std::string& text, const TextStyle& preferredstyle) :
	_text(text)
{
	if (text.empty()) return;

	TextStyle style = preferredstyle;

	Color color = (style.outline > 0) ? style.outlinecolor : style.textcolor;
	SDL_Surface* surface = TTF_RenderUTF8_Blended(
		Graphics::get()->getFont(style.font, style),
		text.c_str(),
		{color.r, color.g, color.b, color.a});
	if (!surface && style.outline > 0)
	{
		LOGW << "TTF generated invalid surface: " << TTF_GetError();
		LOGW << "TTF generated invalid surface while constructing Text,"
			" perhaps invalid UTF8 or unsupported glyphs in"
			" ===" << hexify(text) << "==="
			" \"\"\"" << text << "\"\"\"";
		// Try again without outline.
		style.outline = 0;
		surface = TTF_RenderUTF8_Blended(
			Graphics::get()->getFont(style.font, style),
			text.c_str(),
			{color.r, color.g, color.b, color.a});
	}
	if (!surface)
	{
		LOGW << "TTF generated invalid surface: " << TTF_GetError();
		LOGW << "TTF generated invalid surface while constructing Text,"
			" perhaps invalid UTF8 or unsupported glyphs in"
			" ===" << hexify(text) << "==="
			" \"\"\"" << text << "\"\"\"";
		DEBUG_ASSERT(false);
		return;
	}
	if (style.outline > 0)
	{
		color = style.textcolor;
		FontStyle fgstyle = style;
		fgstyle.outline = 0;
		SDL_Surface* fgsurface = TTF_RenderUTF8_Blended(
			Graphics::get()->getFont(style.font, fgstyle),
			text.c_str(),
			{color.r, color.g, color.b, color.a});
		if (!fgsurface)
		{
			LOGW << "TTF generated invalid surface: " << TTF_GetError();
			LOGW << "TTF generated invalid surface while constructing Text,"
				" perhaps invalid UTF8 or unsupported glyphs in"
				" ===" << hexify(text) << "==="
				" \"\"\"" << text << "\"\"\"";
			DEBUG_ASSERT(false);
			return;
		}
		SDL_Rect rect{style.outline, style.outline, fgsurface->w, fgsurface->h};
		SDL_SetSurfaceBlendMode(fgsurface, SDL_BLENDMODE_BLEND);
		SDL_BlitSurface(fgsurface, nullptr, surface, &rect);
		SDL_FreeSurface(fgsurface);
	}
	if (!surface)
	{
		LOGW << "TTF generated invalid surface while constructing Text";
		DEBUG_ASSERT(false);
		return;
	}
	if (surface->format->format != SDL_PIXELFORMAT_RGBA8888)
	{
		SDL_Surface* newsurface = SDL_ConvertSurfaceFormat(surface,
			SDL_PIXELFORMAT_RGBA8888, 0);
		SDL_FreeSurface(surface);
		surface = newsurface;
	}
	if (!surface)
	{
		LOGW << "SDL generated invalid surface while constructing Text";
		DEBUG_ASSERT(false);
		return;
	}
	_width = surface->w;
	_height = surface->h;
	SDL_Surface* padded = SDL_CreateRGBSurfaceWithFormat(0,
		nearestPowerOfTwo(_width), nearestPowerOfTwo(_height),
		surface->format->BitsPerPixel, surface->format->format);
	if (!padded)
	{
		LOGW << "SDL generated invalid padded while constructing Text";
		DEBUG_ASSERT(false);
		return;
	}
	SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);
	SDL_BlitSurface(surface, nullptr, padded, nullptr);
	_s = float(_width) / padded->w;
	_t = float(_height) / padded->h;

	glGenTextures(1, &_textureID);
	glBindTexture(GL_TEXTURE_2D, _textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, surface->format->BytesPerPixel, padded->w,
		padded->h, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, padded->pixels);
	SDL_FreeSurface(surface);
	SDL_FreeSurface(padded);
}

Text::~Text()
{
	if (_text.empty()) return;

	glDeleteTextures(1, &_textureID);
}

void Text::render(GLuint shader, int x, int y)
{
	if (_text.empty()) return;

	/*
	Once per shader, we declare that the shader variable named 'texture'
	corresponds to GL_TEXTURE0.
	Something similar also happens in the other ::render() functions,
	and it is important that we do not have to reassign 'texture'
	here after it was used by something else.
	*/
	if (shader != _lastShader_static)
	{
		GLuint glTexture = glGetUniformLocation(shader, "texture");
		glUniform1i(glTexture, 0);

		_lastShader_static = shader;
	}

	GLuint glGrayed = glGetUniformLocation(shader, "alpha");
	glUniform1f(glGrayed, _alpha);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _textureID);

	glBegin(GL_QUADS);
	{
		glTexCoord2f(0, 0);
		glVertex2i(x, y);
		glTexCoord2f(_s, 0);
		glVertex2i(x + _width, y);
		glTexCoord2f(_s, _t);
		glVertex2i(x + _width, y + _height);
		glTexCoord2f(0, _t);
		glVertex2i(x, y + _height);
	}
	glEnd();
}

int Text::fontH(int fontsize)
{
	int h = TTF_FontHeight(Graphics::get()->getFont(FontStyle(fontsize)));
	// The default fontstyle has no outline.
	return h;
}

int Text::fontH(const TextStyle& style)
{
	int h = TTF_FontHeight(Graphics::get()->getFont(style.font, style));
	// The TTF_FontHeight method ignores outline, so we need to manually add it
	// to the top and bottom. Outlines are unscaled.
	h += 2 * style.outline;
	return h;
}

int Text::calculateWidth(const TextStyle& style, const std::string& text)
{
	int w = 0;
	int error = TTF_SizeUTF8(Graphics::get()->getFont(style.font, style),
		text.c_str(), &w, nullptr);
	if (error)
	{
		LOGW << "TTF sizing failed: " << TTF_GetError();
		LOGW << "TTF sizing failed,"
			" perhaps invalid UTF8 or unsupported glyphs in"
			" ===" << hexify(text) << "==="
			" \"\"\"" << text << "\"\"\"";
	}
	return w;
}

std::string hexify(const std::string& text)
{
	std::stringstream strm;
	strm << std::hex << std::setfill('0');
	for (char x : text)
	{
		strm << std::setw(2) << (unsigned int) (uint8_t) x;
	}
	return strm.str();
}
