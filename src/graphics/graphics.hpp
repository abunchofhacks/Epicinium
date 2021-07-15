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

struct SDL_Window;
typedef void* SDL_GLContext;
typedef struct _TTF_Font TTF_Font;

class Settings;
struct FontStyle;

enum class Font : uint8_t;
enum class Shader : uint8_t;


class Graphics
{
private:
	static Graphics* _installed;

public:
	static Graphics* get() { return _installed; }

	Graphics(Settings& settings);
	Graphics(const Graphics&) = delete;
	Graphics(Graphics&&) = delete;
	Graphics& operator=(const Graphics&) = delete;
	Graphics& operator=(Graphics&&) = delete;
	~Graphics();

private:
	SDL_Window* _window;
	SDL_GLContext _context;
	std::vector<Shader> _shaders;        // married
	std::vector<GLuint> _shaderPrograms; // married
	size_t _shaderindexSprite;
	size_t _shaderindexPicture;
	size_t _shaderindexText;
	std::vector<std::string> _installedFontFilenames;
	std::vector<Font> _fonts;             // married
	std::vector<FontStyle> _fontStyles;   // married
	std::vector<TTF_Font*> _fontTextures; // married
	Settings& _settings;

	unsigned int _rendertimeoffset = 0;
	unsigned int _renderstarttime = 0;
	float _rendertime[10] = {0};
	bool _finish = false;
	bool _debugUI = false;

	void init();
	void loadShader(Shader shader);

public:
	void install();

	SDL_Window* getWindow() { return _window; }

	GLuint getSpriteShader() const
	{
		return _shaderPrograms[_shaderindexSprite];
	}
	GLuint getPictureShader() const
	{
		return _shaderPrograms[_shaderindexPicture];
	}
	GLuint getTextShader() const
	{
		return _shaderPrograms[_shaderindexText];
	}
	GLuint getPrimitiveShader() const
	{
		return _shaderPrograms[_shaderindexPicture];
	}

	Font findFontFromFilename(const std::string& filename);
	TTF_Font* getFont(const FontStyle& style);
	TTF_Font* getFont(const Font& font, const FontStyle& style);
	int width();
	int height();
	std::vector<std::string> resolutions();
	bool debugUI() const { return _debugUI; }

	void clear();
	void flip();
	void prepare();
	void update();
	void finish();

	void raiseWindow();
	void resetRenderTarget();
};
