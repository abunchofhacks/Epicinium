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
#include "picture.hpp"
#include "source.hpp"

#include "color.hpp"
#include "texture.hpp"
#include "camera.hpp"
#include "loop.hpp"
#include "spritepattern.hpp"
#include "locator.hpp"


GLuint Picture::_lastShader_static = 0;

Picture::Picture(const Texture* t, const std::string& picturename) :
	_missing(t->_filename != Locator::pictureFilename(picturename)),
	_nameOfPicture(picturename),
	_texture(t),
	_thetaOffset((rand() % 36000) * 0.01f)
{}

Picture::Picture(const std::string& pname) :
	Picture(Texture::picture(Locator::picture(pname)), pname)
{}

Picture::~Picture()
{}

void Picture::render(GLuint shader, int x, int y)
{
	/*
	Once per shader, we declare that the shader variable named 'texture'
	corresponds to GL_TEXTURE0 and 'mask' to GL_TEXTURE2.
	Something similar also happens in the other ::render() functions,
	and it is important that we do not have to reassign 'texture'
	here after it was used by something else.
	*/
	if (shader != _lastShader_static)
	{
		GLuint glTexture = glGetUniformLocation(shader, "texture");
		glUniform1i(glTexture, 0);

		GLuint glMask = glGetUniformLocation(shader, "mask");
		glUniform1i(glMask, 2);

		_lastShader_static = shader;
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _texture->_textureID);

	GLuint glObscured = glGetUniformLocation(shader, "obscured");
	glUniform1f(glObscured, 0);

	GLuint glGrayed = glGetUniformLocation(shader, "grayed");
	glUniform1f(glGrayed, _grayed);

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
	glUniform1i(glMasked, _isSetAsBackground);

	GLuint glBorder = glGetUniformLocation(shader, "border");
	glUniform1i(glBorder, false);

	if (_isSetAsBackground) drawBackgroundQuads(shader, x, y);
	else drawStandardQuads(shader, x, y);
}

void Picture::drawStandardQuads(GLuint /**/, int x, int y)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float color[] = {0.0f, 0.0f, 0.0f, 0.0f};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

	// For pictures, the x and y are the top left of the picture.
	int l = x;
	int r = x + scaledWidth();
	int t = y;
	int b = y + scaledHeight();

	// If the image smaller than the view, we repeat. Otherwise we crop.
	int scale = Camera::get()->SCALE;
	float wfactor = 1.0f * scaledWidth()  / (scale * _texture->_width );
	float hfactor = 1.0f * scaledHeight() / (scale * _texture->_height);

	// Calculate the corresponding coordinates on the texture.
	float tl = 0.5f - 0.5f * wfactor
		+ (1.0f * _xOffset / (scale * _texture->_width));
	float tr = tl + wfactor;
	float tt = 0.5f - 0.5f * hfactor
		+ (1.0f * _yOffset / (scale * _texture->_height));
	float tb = tt + hfactor;

	glBegin(GL_QUADS);
	{
		glTexCoord2f(tl, tt);
		glVertex2i(l, t);
		glTexCoord2f(tr, tt);
		glVertex2i(r, t);
		glTexCoord2f(tr, tb);
		glVertex2i(r, b);
		glTexCoord2f(tl, tb);
		glVertex2i(l, b);
	}
	glEnd();
}

void Picture::drawBackgroundQuads(GLuint /**/, int x, int y)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	int scale = Camera::get()->SCALE;

	// We calculate the inner patch size of the sprite we are masked by now.
	int innerPatchWidth = scaledWidth()
		- _spriteMask->leftPatchWidth() * scale
		- _spriteMask->rightPatchWidth() * scale;
	int innerPatchHeight = scaledHeight()
		- _spriteMask->topPatchHeight() * scale
		- _spriteMask->bottomPatchHeight() * scale;

	// x and y are the coordinates of the upper left corner,
	// excluding the texture's margin
	int xx = x;
	GLint vertices_x[] = {
		xx,
		(xx += _spriteMask->leftPatchWidth() * scale),
		(xx += innerPatchWidth),
		(xx += _spriteMask->rightPatchWidth() * scale),
	};
	int yy = y;
	GLint vertices_y[] = {
		yy,
		(yy += _spriteMask->topPatchHeight() * scale),
		(yy += innerPatchHeight),
		(yy += _spriteMask->bottomPatchHeight() * scale),
	};
	float mx = 0;
	GLfloat masks_x[] = {
		(mx += 1.0f * _spriteMask->leftMarginWidth() / _spriteMask->width()),
		(mx += 1.0f * _spriteMask->leftPatchWidth() / _spriteMask->width()),
		(mx += 1.0f * _spriteMask->innerPatchWidth() / _spriteMask->width()),
		(mx += 1.0f * _spriteMask->rightPatchWidth() / _spriteMask->width()),
	};
	float my = 0;
	GLfloat masks_y[] = {
		(my += 1.0f * _spriteMask->topMarginHeight() / _spriteMask->height()),
		(my += 1.0f * _spriteMask->topPatchHeight() / _spriteMask->height()),
		(my += 1.0f * _spriteMask->innerPatchHeight() / _spriteMask->height()),
		(my += 1.0f * _spriteMask->bottomPatchHeight() / _spriteMask->height()),
	};

	// If the image smaller than the view, we repeat. Otherwise we crop.
	float wfactor = 1.0f * scaledWidth()  / (scale * _texture->_width );
	float hfactor = 1.0f * scaledHeight() / (scale * _texture->_height);

	// Calculate the corresponding coordinates on the mask.
	float tx = 0.5f - 0.5f * wfactor;
	GLfloat coords_x[] = {
		tx,
		(tx += wfactor * _spriteMask->leftPatchWidth() * scale / scaledWidth()),
		(tx += wfactor * innerPatchWidth / scaledWidth()),
		(tx += wfactor * _spriteMask->rightPatchWidth() * scale / scaledWidth()),
	};
	float ty = 0.5f - 0.5f * hfactor;
	GLfloat coords_y[] = {
		ty,
		(ty += hfactor * _spriteMask->topPatchHeight() * scale / scaledHeight()),
		(ty += hfactor * innerPatchHeight / scaledHeight()),
		(ty += hfactor * _spriteMask->bottomPatchHeight() * scale / scaledHeight()),
	};

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, _spriteMask->texture());

	glBegin(GL_QUADS);
	for (int r = 0; r < 3; r++)
	{
		for (int c = 0; c < 3; c++)
		{
			glMultiTexCoord2f(GL_TEXTURE0, coords_x[c + 0],   coords_y[r + 0]);
			glMultiTexCoord2f(GL_TEXTURE2,  masks_x[c + 0],    masks_y[r + 0]);
			glVertex2i(                  vertices_x[c + 0], vertices_y[r + 0]);
			glMultiTexCoord2f(GL_TEXTURE0, coords_x[c + 1],   coords_y[r + 0]);
			glMultiTexCoord2f(GL_TEXTURE2,  masks_x[c + 1],    masks_y[r + 0]);
			glVertex2i(                  vertices_x[c + 1], vertices_y[r + 0]);
			glMultiTexCoord2f(GL_TEXTURE0, coords_x[c + 1],   coords_y[r + 1]);
			glMultiTexCoord2f(GL_TEXTURE2,  masks_x[c + 1],    masks_y[r + 1]);
			glVertex2i(                  vertices_x[c + 1], vertices_y[r + 1]);
			glMultiTexCoord2f(GL_TEXTURE0, coords_x[c + 0],   coords_y[r + 1]);
			glMultiTexCoord2f(GL_TEXTURE2,  masks_x[c + 0],    masks_y[r + 1]);
			glVertex2i(                  vertices_x[c + 0], vertices_y[r + 1]);
		}
	}
	glEnd();
}

void Picture::setAsBackgroundOf(const SpritePattern* pattern)
{
	_isSetAsBackground = true;
	_spriteMask = pattern;
}

void Picture::setOffset(int dx, int dy)
{
	_xOffset = dx;
	_yOffset = dy;
}

void Picture::setWidth(int w)
{
	_viewWidth = w;
}

void Picture::setHeight(int h)
{
	_viewHeight = h;
}

void Picture::setGrayed(float grayed)
{
	_grayed = grayed;
}

void Picture::setShine(float shine)
{
	_shine = shine;
}

void Picture::setShineColor(const Color& color)
{
	_shinecolor[0] = color.r / 255.0;
	_shinecolor[1] = color.g / 255.0;
	_shinecolor[2] = color.b / 255.0;
}

void Picture::setThetaOffset(float offset)
{
	_thetaOffset = offset;
}

int Picture::width() const
{
	return _texture->_width;
}

int Picture::height() const
{
	return _texture->_height;
}

int Picture::scaledWidth() const
{
	if (_viewWidth) return _viewWidth;
	return width() * Camera::get()->SCALE;
}

int Picture::scaledHeight() const
{
	if (_viewHeight) return _viewHeight;
	return height() * Camera::get()->SCALE;
}

int Picture::topleftXenon(int x) const
{
	return x - _xOffset * Camera::get()->SCALE;
}

int Picture::topleftYahoo(int y) const
{
	return y - _yOffset * Camera::get()->SCALE;
}
