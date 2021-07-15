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
#include "screenshot.hpp"
#include "source.hpp"

#include "graphics.hpp"


Screenshot::Screenshot(int width, int height, const std::string& tag,
		bool renderable) :
	_tag(tag),
	_width(width),
	_height(height)
{
	LOGI << "Taking screenshot...";

	glGenTextures(1, &_textureID);
	glBindTexture(GL_TEXTURE_2D, _textureID);
	// Set mipmap level (currently 0) as a cheap way to zoom out?
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
		_width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	if (!renderable)
	{
		_framebufferID = 0;
		return;
	}

	glGenFramebuffers(1, &_framebufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, _framebufferID);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
		_textureID, 0);
	GLenum DRAW_BUFFERS[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, DRAW_BUFFERS);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		LOGE << "Failed to prepare framebuffer for screenshot";
	}
}

Screenshot::~Screenshot()
{
	if (_textureID) glDeleteTextures(1, &_textureID);
	if (_framebufferID) glDeleteFramebuffers(1, &_framebufferID);
}

void Screenshot::setAsRenderTarget()
{
	glBindFramebuffer(GL_FRAMEBUFFER, _framebufferID);
	glViewport(0, 0, _width, _height);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, _width, 0, _height, 0, 1);
}

void Screenshot::finishRendering()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glPopMatrix();
}

std::vector<uint8_t> Screenshot::writeToBuffer()
{
	std::vector<uint8_t> buffer;
	buffer.resize(4 * _width * _height);
	glBindTexture(GL_TEXTURE_2D, _textureID);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		buffer.data());
	return buffer;
}
