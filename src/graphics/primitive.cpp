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
#include "primitive.hpp"
#include "source.hpp"

#include "libs/SDL2/SDL2_gfxPrimitives.h"

#include "color.hpp"
#include "camera.hpp"
#include "loop.hpp"


GLuint Primitive::_lastShader_static = 0;

Primitive::Primitive(const Type& type, const Color& color, int w, int h) :
	_type(type),
	_width(w),
	_height(h),
	_thetaOffset((rand() % 36000) * 0.01f)
{
	SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0,
		nearestPowerOfTwo(_width), nearestPowerOfTwo(_height),
		32, SDL_PIXELFORMAT_RGBA8888);
	if (!surface)
	{
		LOGW << "SDL generated invalid surface while constructing Primitive";
		DEBUG_ASSERT(false);
		return;
	}
	_s = float(_width) / surface->w;
	_t = float(_height) / surface->h;

	SDL_Renderer* renderer = SDL_CreateSoftwareRenderer(surface);
	if (!renderer)
	{
		LOGW << "SDL generated invalid renderer while constructing Primitive";
		DEBUG_ASSERT(false);
		return;
	}

	if (w != 0 && h != 0)
	{
		switch (_type)
		{
			case Type::RECTANGLE_FILLED:
			{
				boxRGBA(renderer,
					0, 0, w - 1, h - 1,
					color.r, color.g, color.b, color.a);
			}
			break;
			case Type::RECTANGLE_OUTLINE:
			{
				rectangleRGBA(renderer,
					0, 0, w - 1, h - 1,
					color.r, color.g, color.b, color.a);
			}
			break;
			// TODO check if this is right if we ever use ellipse again
			case Type::ELLIPSE_FILLED:
			{
				filledEllipseRGBA(renderer,
					(w - 1) / 2,
					(h - 1) / 2,
					(w - 1) / 2,
					(h - 1) / 2,
					color.r, color.g, color.b, color.a);
			}
			break;
			case Type::ELLIPSE_OUTLINE:
			{
				ellipseRGBA(renderer,
					(w - 1) / 2,
					(h - 1) / 2,
					(w - 1) / 2,
					(h - 1) / 2,
					color.r, color.g, color.b, color.a);
			}
			break;
		}
	}

	SDL_DestroyRenderer(renderer);

	glGenTextures(1, &_textureID);
	glBindTexture(GL_TEXTURE_2D, _textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, surface->format->BytesPerPixel, surface->w,
		surface->h, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, surface->pixels);
	SDL_FreeSurface(surface);
}

Primitive::~Primitive()
{
	glDeleteTextures(1, &_textureID);
}

void Primitive::render(GLuint shader, int x, int y)
{
	if (shader != _lastShader_static)
	{
		GLuint glTexture = glGetUniformLocation(shader, "texture");
		glUniform1i(glTexture, 0);

		_lastShader_static = shader;
	}

	GLuint glObscured = glGetUniformLocation(shader, "obscured");
	glUniform1f(glObscured, _obscured);

	GLuint glGrayed = glGetUniformLocation(shader, "grayed");
	glUniform1f(glGrayed, 0);

	GLuint glShine = glGetUniformLocation(shader, "shine");
	glUniform1f(glShine, _shine);

	if (_shine > 0)
	{
		GLuint glShinecolor = glGetUniformLocation(shader, "shinecolor");
		glUniform3fv(glShinecolor, 1, _shinecolor);
	}

	GLuint glTheta = glGetUniformLocation(shader, "theta");
	glUniform1f(glTheta, fmod(Loop::theta() + _thetaOffset, 360.0f));

	GLuint glMasked = glGetUniformLocation(shader, "masked");
	glUniform1i(glMasked, false);

	GLuint glBorder = glGetUniformLocation(shader, "border");
	glUniform1i(glBorder, false);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _textureID);

	int scale = Camera::get()->SCALE;
	int x0 = x;
	int x1 = x0 + scale * _width;
	int y0 = y;
	int y1 = y0 + scale * _height;

	glBegin(GL_QUADS);
	{
		glTexCoord2f(0, 0);
		glVertex2i(x0, y0);
		glTexCoord2f(_s, 0);
		glVertex2i(x1, y0);
		glTexCoord2f(_s, _t);
		glVertex2i(x1, y1);
		glTexCoord2f(0, _t);
		glVertex2i(x0, y1);
	}
	glEnd();
}

void Primitive::setObscured(float obscured)
{
	_obscured = obscured;
}

void Primitive::setShine(float shine)
{
	_shine = shine;
}

void Primitive::setShineColor(const Color& color)
{
	_shinecolor[0] = color.r / 255.0;
	_shinecolor[1] = color.g / 255.0;
	_shinecolor[2] = color.b / 255.0;
}

void Primitive::setThetaOffset(float offset)
{
	_thetaOffset = offset;
}
