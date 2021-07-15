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
#include "sprite.hpp"
#include "source.hpp"

#include "pixel.hpp"
#include "spritepattern.hpp"
#include "camera.hpp"
#include "loop.hpp"


GLuint Sprite::_lastShader_static = 0;

Sprite::Sprite(SpritePattern* ptn, std::shared_ptr<Palette> palette) :
	_name(ptn->name()),
	_pattern(ptn),
	_palette(palette
		// All particles of the same type share a singular unchanging palette.
		? palette
		// Other sprites all have their own palette.
		: std::make_shared<Palette>(paletteSize(/*_pattern*/))
	),
	_start(0),
	_end(_pattern->slices()),
	_thetaOffset((rand() % 36000) * 0.01f)
{
	_totalduration = 0.0f;
	for (size_t i = _start; i < _end; i++)
	{
		_totalduration += _pattern->slice(i).duration;
	}
}

Sprite::Sprite(const std::string& name, std::shared_ptr<Palette> palette) :
	Sprite(SpritePattern::get(name), palette)
{}

Sprite::~Sprite()
{}

int Sprite::paletteSize() const
{
	// Externally, the "palette size" is the number of colors that we
	// want to be able to assign using setColor, i.e. not including
	// the transparent color. So if we want to call setColor(5),
	// then we want to set colors 0, 1, 2, 3, 4 and 5, i.e. 6 colors.
	// Internally, i.e. within Sprite::Palette and within Texture,
	// color 0 is actually the transparent color, and setColor(0, CLR)
	// actually alters color 1. If our Texture would feature indices
	// 0, 1, 2, 3, 4, 5 and 6, then the highest occurring index is 6.
	// Thus two separate off-by-one relations both lead to the same number,
	// so the number of declarable colors is exactly the highest index.
	return _pattern->maxPaletteIndex();
}

static inline uint8_t smallestPowerOfTwo(size_t count)
{
	// Let x be the smallest power of 2 such that count + 1 <= x,
	// but no more than 256, then the maximum index is x - 1.
	// Also x is at least 4 because of Surface::boom() and ::impact().
	// TODO ugh
	size_t x = 4;
	for (int i = 0; i < 6; i++)
	{
		// if (count + 1 <= x)
		if (count < x) break;
		x *= 2;
	}
	return x - 1;
}

Sprite::Palette::Palette(size_t count) :
	//_id(/* initialized by glGenTextures() */),
	_dirty(true),
	_max(smallestPowerOfTwo(count)),
	_data(((size_t) _max) + 1)
{
	_data[0] = Color::transparent();

	glGenTextures(1, &_id);
	glBindTexture(GL_TEXTURE_1D, _id);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

Sprite::Palette::~Palette()
{
	glDeleteTextures(1, &_id);
}

void Sprite::Palette::bind(GLuint shader)
{
	const size_t size = ((size_t) _max) + 1;

	glBindTexture(GL_TEXTURE_1D, _id);
	if (_dirty || shader != _lastShader)
	{
		glTexImage1D(GL_TEXTURE_1D, 0, 4, size, 0, GL_RGBA, GL_UNSIGNED_BYTE,
			_data.data());

		_dirty = false;
		_lastShader = shader;
	}

	GLuint glPaletteScale = glGetUniformLocation(shader, "palettescale");
	glUniform1f(glPaletteScale, 256 / size);
}

void Sprite::render(GLuint shader, int x, int y, int flags)
{
	/*
	Once per shader, we declare that the shader variable named 'texture'
	corresponds to GL_TEXTURE0, variable 'palette' to GL_TEXTURE1 and
	'mask' to GL_TEXTURE2.
	Something similar also happens in the other ::render() functions,
	and it is important that we do not have to reassign 'texture' or 'palette'
	here after it was used by something else.
	*/
	if (shader != _lastShader_static)
	{
		GLuint glTexture = glGetUniformLocation(shader, "texture");
		glUniform1i(glTexture, 0);

		GLuint glPalette = glGetUniformLocation(shader, "palette");
		glUniform1i(glPalette, 1);

		GLuint glMask = glGetUniformLocation(shader, "mask");
		glUniform1i(glMask, 2);

		_lastShader_static = shader;
	}

	glActiveTexture(GL_TEXTURE1);
	_palette->bind(shader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _pattern->texture(_drawframe));

	GLuint glObscured = glGetUniformLocation(shader, "obscured");
	glUniform1f(glObscured, _obscured);

	GLuint glGrayed = glGetUniformLocation(shader, "grayed");
	glUniform1f(glGrayed, 0.0f);

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
	glUniform1i(glMasked, _blended && !(flags & SurfaceDrawFlag::UNMASKED));

	GLuint glBorder = glGetUniformLocation(shader, "border");
	glUniform1i(glBorder, _border);

	if (_isSetAsBackground) drawBackgroundQuads(shader, x, y);
	else if (_blended) drawBlendedQuads(shader, x, y, flags);
	else if (_ninepatch) drawNinePatchQuads(shader, x, y);
	else drawStandardQuads(shader, x, y);
}

void Sprite::drawBackgroundQuads(GLuint /**/, int x, int y)
{
	// The unit might be invisible.
	if (_drawframe >= _pattern->slices()) return;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// The x and y are the on-screen coordinates where the origin should be
	// placed. By default, the origin is the upper left corner of the pattern,
	// but it can be moved around by setting the offset.
	int scale = drawscale();
	const Slice& slice = _pattern->slice(_drawframe);
	int l = x + slice.x * scale - _xOffset * scale
		- _pattern->leftMarginWidth() * scale;
	int r = l + slice.w * scale;
	int t = y + slice.y * scale - _yOffset * scale
		- _pattern->topMarginHeight() * scale;
	int b = t + slice.h * scale;

	// We cover the entire window with this texture.
	int wl = 0;
	int wt = 0;
	int wr = Camera::get()->width();
	int wb = Camera::get()->height();
	float s1 = 1.0f * (wl - l) / (r - l);
	float t1 = 1.0f * (wt - t) / (b - t);
	float s2 = 1.0f * (wr - l) / (r - l);
	float t2 = 1.0f * (wb - t) / (b - t);

	glBegin(GL_QUADS);
	{
		glTexCoord2f(s1, t1);
		glVertex2i(wl, wt);
		glTexCoord2f(s2, t1);
		glVertex2i(wr, wt);
		glTexCoord2f(s2, t2);
		glVertex2i(wr, wb);
		glTexCoord2f(s1, t2);
		glVertex2i(wl, wb);
	}
	glEnd();
}

void Sprite::drawBlendedQuads(GLuint shader, int x, int y, int flags)
{
	// The unit might be invisible.
	if (_drawframe >= _pattern->slices()) return;

	// The x and y are the on-screen coordinates where the origin should be
	// placed. By default, the origin is the upper left corner of the pattern,
	// but it can be moved around by setting the offset.
	int scale = drawscale();
	const Slice& slice = _pattern->slice(_drawframe);
	int l = x + slice.x * scale - _xOffset * scale
		- _pattern->leftMarginWidth() * scale;
	int r = l + slice.w * scale;
	int t = y + slice.y * scale - _yOffset * scale
		- _pattern->topMarginHeight() * scale;
	int b = t + slice.h * scale;

	// Calculate center, middle, far left, far right, far bottom and far top.
	int c = (l + r) / 2;
	int m = (t + b) / 2;
	int fl = l - slice.w * scale / 2;
	int fr = r + slice.w * scale / 2;
	int ft = t - slice.h * scale / 2;
	int fb = b + slice.h * scale / 2;

	if (flags & SurfaceDrawFlag::UNMASKED)
	{
		// Sander: purple-outlined triangles.
		glBegin(GL_TRIANGLES);
		{
			// Sander: green-and-orange-filled triangle.
			glTexCoord2f(slice.right, slice.top);
			glVertex2i(r, t);
			glTexCoord2f(slice.right, slice.bottom);
			glVertex2i(r, b);
			glTexCoord2f(slice.left, slice.bottom);
			glVertex2i(l, b);

			// Sander: red-filled triangle.
			glTexCoord2f(slice.left, slice.top);
			glVertex2i(r, t);
			glTexCoord2f(slice.center, slice.middle);
			glVertex2i(fr, m);
			glTexCoord2f(slice.left, slice.bottom);
			glVertex2i(r, b);

			// Sander: blue-filled triangle.
			glTexCoord2f(slice.right, slice.top);
			glVertex2i(r, b);
			glTexCoord2f(slice.center, slice.middle);
			glVertex2i(c, fb);
			glTexCoord2f(slice.left, slice.top);
			glVertex2i(l, b);
		}
		glEnd();
	}

	if (   (flags & SurfaceDrawFlag::MASKED)
		&& (flags & SurfaceDrawFlag::UNMASKED))
	{
		GLuint glMasked = glGetUniformLocation(shader, "masked");
		glUniform1i(glMasked, true);
	}

	if (flags & SurfaceDrawFlag::MASKED)
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, _blendTop->texture());

		// Calculate the corresponding coordinates on the mask.
		// TODO animated masks require non-trivial coordinates
		float ml = 0.0f;
		float mr = 1.0f;
		float mt = 0.0f;
		float mb = 1.0f;
		float mc = (ml + mr) / 2;
		float mm = (mt + mb) / 2;

		// Sander: blue- and orange-outlined triangles.
		glBegin(GL_TRIANGLES);
		{
			// Sander: blue-filled triangle.
			glMultiTexCoord2f(GL_TEXTURE0, slice.left, slice.top);
			glMultiTexCoord2f(GL_TEXTURE2, ml, mt);
			glVertex2i(l, t);
			glMultiTexCoord2f(GL_TEXTURE0, slice.right, slice.top);
			glMultiTexCoord2f(GL_TEXTURE2, mr, mt);
			glVertex2i(r, t);
			glMultiTexCoord2f(GL_TEXTURE0, slice.center, slice.middle);
			glMultiTexCoord2f(GL_TEXTURE2, mc, mm);
			glVertex2i(c, m);

			// Sander: orange-filled triangle.
			glMultiTexCoord2f(GL_TEXTURE0, slice.center, slice.middle);
			glMultiTexCoord2f(GL_TEXTURE2, mc, mm);
			glVertex2i(c, ft);
			glMultiTexCoord2f(GL_TEXTURE0, slice.right, slice.bottom);
			glMultiTexCoord2f(GL_TEXTURE2, mr, mb);
			glVertex2i(r, t);
			glMultiTexCoord2f(GL_TEXTURE0, slice.left, slice.bottom);
			glMultiTexCoord2f(GL_TEXTURE2, ml, mb);
			glVertex2i(l, t);
		}
		glEnd();

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, _blendLeft->texture());

		// Sander: red- and green-outlined triangles.
		glBegin(GL_TRIANGLES);
		{
			// Sander: red-filled triangle.
			glMultiTexCoord2f(GL_TEXTURE0, slice.left, slice.top);
			glMultiTexCoord2f(GL_TEXTURE2, ml, mt);
			glVertex2i(l, t);
			glMultiTexCoord2f(GL_TEXTURE0, slice.center, slice.middle);
			glMultiTexCoord2f(GL_TEXTURE2, mc, mm);
			glVertex2i(c, m);
			glMultiTexCoord2f(GL_TEXTURE0, slice.left, slice.bottom);
			glMultiTexCoord2f(GL_TEXTURE2, ml, mb);
			glVertex2i(l, b);

			// Sander: green-filled triangle.
			glMultiTexCoord2f(GL_TEXTURE0, slice.right, slice.top);
			glMultiTexCoord2f(GL_TEXTURE2, mr, mt);
			glVertex2i(l, t);
			glMultiTexCoord2f(GL_TEXTURE0, slice.right, slice.bottom);
			glMultiTexCoord2f(GL_TEXTURE2, mr, mb);
			glVertex2i(l, b);
			glMultiTexCoord2f(GL_TEXTURE0, slice.center, slice.middle);
			glMultiTexCoord2f(GL_TEXTURE2, mc, mm);
			glVertex2i(fl, m);
		}
		glEnd();
	}

	if (   !(flags & SurfaceDrawFlag::MASKED)
		&& !(flags & SurfaceDrawFlag::UNMASKED))
	{
		LOGE << "missing flags or incorrect flags: " << flags;
		DEBUG_ASSERT(false);
	}
}

void Sprite::drawStandardQuads(GLuint /**/, int x, int y)
{
	// The unit might be invisible.
	if (_drawframe >= _pattern->slices()) return;

	// The x and y are the on-screen coordinates where the origin should be
	// placed. By default, the origin is the upper left corner of the pattern,
	// but it can be moved around by setting the offset.
	int scale = drawscale();
	const Slice& slice = _pattern->slice(_drawframe);
	int l = x + slice.x * scale - _xOffset * scale
		- _pattern->leftMarginWidth() * scale;
	int r = l + slice.w * scale;
	int t = y + slice.y * scale - _yOffset * scale
		- _pattern->topMarginHeight() * scale;
	int b = t + slice.h * scale;

	glBegin(GL_QUADS);
	{
		glTexCoord2f(slice.left, slice.top);
		glVertex2i(l, t);
		glTexCoord2f(slice.right, slice.top);
		glVertex2i(r, t);
		glTexCoord2f(slice.right, slice.bottom);
		glVertex2i(r, b);
		glTexCoord2f(slice.left, slice.bottom);
		glVertex2i(l, b);
	}
	glEnd();
}

void Sprite::drawNinePatchQuads(GLuint /**/, int x, int y)
{
	int scale = drawscale();

	// The x and y are the on-screen coordinates of the upper left corner,
	// excluding the texture's margin.
	int xx = x;
	GLint vertices_x[] = {
		xx,
		(xx += _pattern->leftPatchWidth() * scale),
		(xx += _innerPatchWidth),
		(xx += _pattern->rightPatchWidth() * scale),
	};
	int yy = y;
	GLint vertices_y[] = {
		yy,
		(yy += _pattern->topPatchHeight() * scale),
		(yy += _innerPatchHeight),
		(yy += _pattern->bottomPatchHeight() * scale),
	};
	float tx = 0;
	GLfloat coords_x[] = {
		(tx += 1.0f * _pattern->leftMarginWidth() / _pattern->width()),
		(tx += 1.0f * _pattern->leftPatchWidth() / _pattern->width()),
		(tx += 1.0f * _pattern->innerPatchWidth() / _pattern->width()),
		(tx += 1.0f * _pattern->rightPatchWidth() / _pattern->width()),
	};
	float ty = 0;
	GLfloat coords_y[] = {
		(ty += 1.0f * _pattern->topMarginHeight() / _pattern->height()),
		(ty += 1.0f * _pattern->topPatchHeight() / _pattern->height()),
		(ty += 1.0f * _pattern->innerPatchHeight() / _pattern->height()),
		(ty += 1.0f * _pattern->bottomPatchHeight() / _pattern->height()),
	};

	glBegin(GL_QUADS);
	for (int r = 0; r < 3; r++)
	{
		for (int c = 0; c < 3; c++)
		{
			glTexCoord2f(coords_x[c + 0], coords_y[r + 0]);
			glVertex2i(  vertices_x[c + 0], vertices_y[r + 0]);
			glTexCoord2f(coords_x[c + 1], coords_y[r + 0]);
			glVertex2i(  vertices_x[c + 1], vertices_y[r + 0]);
			glTexCoord2f(coords_x[c + 1], coords_y[r + 1]);
			glVertex2i(  vertices_x[c + 1], vertices_y[r + 1]);
			glTexCoord2f(coords_x[c + 0], coords_y[r + 1]);
			glVertex2i(  vertices_x[c + 0], vertices_y[r + 1]);
		}
	}
	glEnd();
}

void Sprite::randomizeAnimationStart()
{
	_progress = 0.001f * (rand() % 1000) * _totalduration;
	_nextframe = _start;
	while (_nextframe < _end
		&& _progress > _pattern->slice(_nextframe).duration)
	{
		_progress -= _pattern->slice(_nextframe).duration;
		_nextframe++;
	}
}

void Sprite::setOriginAtBase()
{
	_xOffset = _pattern->width() / 2;
	_yOffset = _pattern->height();
}

void Sprite::setOriginAtCenter()
{
	_xOffset = _pattern->width() / 2;
	_yOffset = _pattern->height() / 2;
}

void Sprite::setOffset(int dx, int dy)
{
	_xOffset += dx;
	_yOffset += dy;
}

void Sprite::setUpscale(int s)
{
	_upscale = s;
	DEBUG_ASSERT(_upscale >= 1 && _upscale <= 256);
}

void Sprite::setBlendTop(const char* blendname)
{
	_blended = true;

	_blendTop = SpritePattern::get(blendname);
}

void Sprite::setBlendLeft(const char* blendname)
{
	_blended = true;

	_blendLeft = SpritePattern::get(blendname);
}

void Sprite::inheritBlends(std::shared_ptr<Sprite> other)
{
	if (!other)
	{
		LOGE << "Other is null";
		DEBUG_ASSERT(false);
		return;
	}

	_blended = other->_blended;
	_blendTop = other->_blendTop;
	_blendLeft = other->_blendLeft;
}

void Sprite::setNinePatch()
{
	_ninepatch = true;
	_pattern->cutNinePatch();

	_emptyWidth = Camera::get()->scale() * (
		+ _pattern->leftPatchWidth()
		+ _pattern->rightPatchWidth());

	_emptyHeight = Camera::get()->scale() * (
		+ _pattern->topPatchHeight()
		+ _pattern->bottomPatchHeight());
}

void Sprite::setAsBackground()
{
	_isSetAsBackground = true;
	_pattern->cutSlicesIntoSeparateTextures();
}

void Sprite::trimMargins()
{
	_pattern->trim();
}

void Sprite::setWidth(int w)
{
	if (w < _emptyWidth) _innerPatchWidth = 0;
	else _innerPatchWidth  = w - _emptyWidth;
}

void Sprite::setHeight(int h)
{
	if (h < _emptyHeight) _innerPatchHeight = 0;
	else _innerPatchHeight = h - _emptyHeight;
}

void Sprite::Palette::setColor(size_t index, const Color& color)
{
	DEBUG_ASSERT(index + 1 <= _max);

	if (_data[index + 1] == color) return;
	_data[index + 1] = color;
	_dirty = true;
}

void Sprite::setColor(size_t index, const Color& color)
{
	_palette->setColor(index, color);
}

const Color& Sprite::Palette::getColor(size_t index)
{
	DEBUG_ASSERT(index + 1 <= _max);

	return _data[index + 1];
}

const Color& Sprite::getColor(size_t index)
{
	return _palette->getColor(index);
}

void Sprite::setObscured(float obscured)
{
	_obscured = obscured;
}

void Sprite::setShine(float shine)
{
	_shine = shine;
}

void Sprite::setShineColor(const Color& color)
{
	_shinecolor[0] = color.r / 255.0;
	_shinecolor[1] = color.g / 255.0;
	_shinecolor[2] = color.b / 255.0;
}

void Sprite::setThetaOffset(float offset)
{
	_thetaOffset = offset;
}

void Sprite::setTag(const std::string& tag)
{
	if (_passive == tag) return;
	const Tag& range = _pattern->tag(tag);
	if (range.end <= range.start) return;
	_passive = tag;
	_start = range.start;
	_end = range.end;
	_nextframe = _start;
	_progress = 0;
	_totalduration = _pattern->getTagDuration(tag);
}

void Sprite::setTagActive(const std::string& tag, bool restart)
{
	if (!restart && _active == tag) return;
	const Tag& range = _pattern->tag(tag);
	if (range.end <= range.start) return;
	_active = tag;
	_start = range.start;
	_end = range.end;
	_nextframe = _start;
	_progress = 0;
	_totalduration = _pattern->getTagDuration(tag);
}

float Sprite::getTagDuration(const std::string& tag)
{
	return _pattern->getTagDuration(tag);
}

void Sprite::setFinal(bool final)
{
	_final = final;
}

void Sprite::setVisible(bool visible)
{
	_visible = visible;
}

void Sprite::setBorder(bool border)
{
	_border = border;
}

const std::string& Sprite::getTag() const
{
	if (!_active.empty()) return _active;
	else return _passive;
}

void Sprite::update()
{
	// The sprite might be invisible.
	if (!_visible)
	{
		_drawframe = _pattern->slices();
		return;
	}

	// Set the frame we will be drawing in the upcoming draw step.
	_drawframe = _nextframe;

	// If the frame is not animated, we never need to change _nextframe.
	if (_pattern->slices() < 2) return;

	// The _drawframe/_nextframe separation fixes an off-by-one error in number
	// of frames. E.g. if a 2-frame active animation takes 2 draw steps per
	// frame, then we want to have four draw steps where _drawframe is 0.0, 0.5,
	// 1.0 and 1.5 respectively. If we directly update _drawframe, we only get
	// 0.5, 1.0 and 1.5 because 2.0 will reset it.

	// Update _nextframe.
	float dt = Loop::delta() * Loop::tempo();
	_progress += dt;
	while (_progress > _pattern->slice(_nextframe).duration)
	{
		_progress -= _pattern->slice(_nextframe).duration;
		_nextframe++;

		if (_nextframe >= _end)
		{
			// Is this a final animation (e.g. death animation)?
			if (_final)
			{
				// No longer draw the sprite after this draw step.
				setVisible(false);
				return;
			}
			else if (!_active.empty())
			{
				// Return to the passive animation.
				_active = "";
				const Tag& tag = _pattern->tag(_passive);
				_start = tag.start;
				_end = tag.end;
				_nextframe = _start;
				_progress = 0;
				_totalduration = _pattern->getTagDuration(_passive);
			}
			else
			{
				// Animate the passive animation in a loop.
				_nextframe = _start;
			}
		}
	}
}

int Sprite::drawscale() const
{
	return _upscale * Camera::get()->scale();
}

int Sprite::width() const
{
	return _pattern->width();
}

int Sprite::height() const
{
	return _pattern->height();
}

int Sprite::scaledWidth() const
{
	if (_ninepatch) return _innerPatchWidth + _emptyWidth;
	else return width() * drawscale();
}

int Sprite::scaledHeight() const
{
	if (_ninepatch) return _innerPatchHeight + _emptyHeight;
	else return height() * drawscale();
}

int Sprite::topleftXenon(int x) const
{
	return x - _xOffset * drawscale();
}

int Sprite::topleftYahoo(int y) const
{
	return y - _yOffset * drawscale();
}

bool Sprite::onMask(const Pixel& origin, const Pixel& pixel) const
{
	// The unit might be invisible.
	if (_drawframe >= _pattern->slices())
	{
		return false;
	}

	// The x and y are the on-screen coordinates where the origin should be
	// placed. By default, the origin is the upper left corner of the pattern,
	// but it can be moved around by setting the offset.
	int scale = drawscale();
	int l = topleftXenon(origin.xenon);
	int r = l + scaledWidth();
	int t = topleftYahoo(origin.yahoo);
	int b = t + scaledHeight();

	// Optimization.
	if (! (l <= pixel.xenon && pixel.xenon < r
		&& t <= pixel.yahoo && pixel.yahoo < b))
	{
		return false;
	}

	// Calculate row and column indices of the mouse pixel on the texture.
	Pixel displacement(pixel.xenon - l, pixel.yahoo - t);
	int col = displacement.xenon;
	int row = displacement.yahoo;

	// For ninepatch sprites, the center may be stretched so we need to
	// unstretch.
	if (_ninepatch)
	{
		col += _pattern->leftMarginWidth() * scale;
		row += _pattern->topMarginHeight() * scale;

		bool center = false;

		int cc = (_pattern->leftMarginWidth() + _pattern->leftPatchWidth()) * scale;
		if (col >= cc + _innerPatchWidth)
		{
			// Part of right patch.
			col -= _innerPatchWidth - _pattern->innerPatchWidth() * scale;
		}
		else if (col >= cc)
		{
			// Part of center patch.
			col -= cc;
			col = col * _pattern->innerPatchWidth() * scale / _innerPatchWidth;
			col += cc;
			center = true;
		}
		else
		{
			// Part of left patch.
		}

		int rr = (_pattern->topMarginHeight() + _pattern->topPatchHeight()) * scale;
		if (row >= rr + _innerPatchHeight)
		{
			// Part of bottom patch.
			row -= _innerPatchHeight - _pattern->innerPatchHeight() * scale;
		}
		else if (row >= rr)
		{
			// Part of middle patch. Optimization: if this is the centermid
			// patch, the mask is always 1.
			if (center) return true;
			row -= rr;
			row = row * _pattern->innerPatchHeight() * scale / _innerPatchHeight;
			row += rr;
		}
		else
		{
			// Part of top patch.
		}
	}

	// For ninepatch sprites the outer 1 pixel margin contains markers that
	// should not be masked.
	if (_ninepatch && (col / scale == 0 || col / scale == _pattern->width()
		||             row / scale == 0 || row / scale == _pattern->height()))
	{
		return false;
	}

	// Ask the pattern.
	return _pattern->onMask(_drawframe, col / scale, row / scale);
}
