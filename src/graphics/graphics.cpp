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
#include "graphics.hpp"
#include "source.hpp"

#include "libs/SDL2/SDL.h"
#include "libs/SDL2/SDL_ttf.h"
#include "libs/SDL2/SDL_image.h"

#include "settings.hpp"
#include "shader.hpp"
#include "font.hpp"
#include "input.hpp"


Graphics* Graphics::_installed = nullptr;

Graphics::Graphics(Settings& settings) :
	_settings(settings),
	_finish(_settings.finishRendering.value(false))
{
	init();
}

Graphics::~Graphics()
{
	for (GLuint program : _shaderPrograms)
	{
		glDeleteProgram(program);
	}
	if (_context) SDL_GL_DeleteContext(_context);
	if (_window) SDL_DestroyWindow(_window);
	for (auto font : _fontTextures)
	{
		TTF_CloseFont(font);
	}
	TTF_Quit();
	IMG_Quit();
}

Font Graphics::findFontFromFilename(const std::string& filename)
{
	// Limit number of fonts to 255 because the Font enum is uint8_t.
	for (uint8_t i = 0; i < 255 && i < _installedFontFilenames.size(); i++)
	{
		if (filename == _installedFontFilenames[i])
		{
			return (Font) i;
		}
	}

	if (_installedFontFilenames.size() >= 255)
	{
		LOGE << "Cannot install more than 255 fonts.";
		DEBUG_ASSERT(false);
		return Font::DEFAULT;
	}
	else
	{
		Font newfont = (Font) (uint8_t) _installedFontFilenames.size();
		_installedFontFilenames.push_back(filename);
		return newfont;
	}
}

TTF_Font* Graphics::getFont(const FontStyle& style)
{
	return getFont(Font::DEFAULT, style);
}

TTF_Font* Graphics::getFont(const Font& font, const FontStyle& style)
{
	// Has this font already been loaded?
	for (size_t i = 0; i < _fonts.size(); i++)
	{
		if (_fonts[i] == font && _fontStyles[i] == style)
		{
			return _fontTextures[i];
		}
	}

	// Determine the filename.
	if (((uint8_t) font) >= _installedFontFilenames.size())
	{
		LOGF << "Invalid font enum " << ((int) (uint8_t) font);
		throw std::runtime_error("Invalid font enum "
				+ std::to_string((int) (uint8_t) font));
	}
	std::string filename = _installedFontFilenames[(uint8_t) font];

	// Load the font.
	TTF_Font* texture = TTF_OpenFont(filename.c_str(), style.size);
	if (!texture)
	{
		LOGF << "Failed to load font " << filename;
		throw std::runtime_error("Failed to load font "
				+ std::string(filename));
	}
	TTF_SetFontHinting(texture, TTF_HINTING_MONO);
	if (style.outline > 0)
	{
		TTF_SetFontOutline(texture, style.outline);
	}
	_fonts.emplace_back(font);
	_fontStyles.emplace_back(style);
	_fontTextures.emplace_back(texture);
	return texture;
}

void Graphics::install()
{
	_installed = this;
}

int Graphics::width()
{
	if (!_window) return 0;
	int w;
	SDL_GL_GetDrawableSize(_window, &w, nullptr);
	return w;
}

int Graphics::height()
{
	if (!_window) return 0;
	int h;
	SDL_GL_GetDrawableSize(_window, nullptr, &h);
	return h;
}

std::vector<std::string> Graphics::resolutions()
{
	std::vector<std::string> result;
	if (!_window) return result;
	int displayIndex = SDL_GetWindowDisplayIndex(_window);
	for (int i = 0; i < SDL_GetNumDisplayModes(displayIndex); i++)
	{
		SDL_DisplayMode mode;
		SDL_GetDisplayMode(displayIndex, i, &mode);
		if (mode.w < 1024) continue;
		if (mode.h < 720) continue;
		std::string res = std::to_string(mode.w) + "x" + std::to_string(mode.h);
		if (std::find(result.begin(), result.end(), res) != result.end()) continue;
		result.emplace_back(res);
	}
	std::reverse(result.begin(), result.end());
	return result;
}

static void GLAPIENTRY handleOpenglError(
	GLenum source, GLenum type, GLuint, GLenum severity,
	GLsizei length, const GLchar* message, const void*)
{
	static size_t totalcount = 0;
	static size_t skipcount = 0;
	static GLsizei lastlength = 0;

	if (type == GL_DEBUG_TYPE_ERROR)
	{
		totalcount += 1;

		if (length < 0) length = strlen(message);

		// Prevent thousands of similar errors (where "similar" here just means
		// with equal error message length as a quick and dirty hash) from
		// completely flooding the logs.
		if (length == lastlength && totalcount > 1000 && skipcount < 100)
		{
			skipcount += 1;
			return;
		}
		else if (skipcount > 0)
		{
			LOGE << "(Skipped " << skipcount << " similar OpenGL errors.)";
		}

		skipcount = 0;
		lastlength = length;

		LOGE << "OpenGL error "
			<< " with severity "
			<< std::hex << std::showbase << severity << std::dec
			<< " from source "
			<< std::hex << std::showbase << source << std::dec
			<< ": \"" << message << "\"";
	}
#ifdef DEVELOPMENT
	else if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
	{
		LOGI << "OpenGL debug of type "
			<< std::hex << std::showbase << type << std::dec
			<< " with severity "
			<< std::hex << std::showbase << severity << std::dec
			<< " from source "
			<< std::hex << std::showbase << source << std::dec
			<< ": \"" << message << "\"";
	}
#endif
}

void Graphics::init()
{
	// Determine the screenmode (windowed, borderless or fullscreen).
	int smode = 0;
	switch (_settings.screenmode.value())
	{
		case ScreenMode::WINDOWED:   smode = 0;                              break;
		case ScreenMode::BORDERLESS: smode = SDL_WINDOW_BORDERLESS;          break;
		case ScreenMode::FULLSCREEN: smode = 0;                              break;
		case ScreenMode::DESKTOP:    smode = SDL_WINDOW_FULLSCREEN_DESKTOP;  break;
	}

	// Create an SDL window which is centered horizontally and vertically.
	// SDL_WINDOW_OPENGL makes it usable with OpenGL context.
	SDL_ClearError();
	_window = SDL_CreateWindow(
		// TRANSLATORS: This is the name of the game. Ideally it is left
		// untranslated.
		_("Epicinium"),
		_settings.windowX.value() >= 0
			? _settings.windowX.value()
			: SDL_WINDOWPOS_CENTERED_DISPLAY(_settings.display.value() - 1),
		_settings.windowY.value() >= 0
			? _settings.windowY.value()
			: SDL_WINDOWPOS_CENTERED_DISPLAY(_settings.display.value() - 1),
		_settings.width.value(), _settings.height.value(),
		SDL_WINDOW_OPENGL | smode);
	if (!_window)
	{
		LOGE << "SDL_CreateWindow failed: " << SDL_GetError();
		fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
	}
	else if (_settings.screenmode.value() == ScreenMode::FULLSCREEN)
	{
		SDL_SetWindowFullscreen(_window, SDL_WINDOW_FULLSCREEN);
	}

	// Create an OpenGL context for the window, so we can use OpenGL methods
	// directly.
	SDL_ClearError();
	_context = SDL_GL_CreateContext(_window);
	if (_context == nullptr)
	{
		LOGE << "SDL_GL_CreateContext failed: " << SDL_GetError();
		fprintf(stderr, "SDL_GL_CreateContext failed: %s\n", SDL_GetError());
	}

	// Initialize the OpenGL Extension Wrangler. This loads supported extensions
	// at runtime so we can use things like shaders without loading extensions
	// manually, the method to which may differ per platform.
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		LOGE << "glewInit failed: " << glewGetErrorString(err);
		fprintf(stderr, "glewInit failed: %s\n", glewGetErrorString(err));
	}

	// Enable OpenGL debug error reporting.
	LOGD << "OpenGL version: " << glGetString(GL_VERSION);
	glEnable(GL_DEBUG_OUTPUT);
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		if (err == GL_INVALID_ENUM) continue;

		LOGW << "OpenGL error: " << err;
#ifdef DEVELOPMENT
		fprintf(stderr, "OpenGL error: %d\n", err);
#endif
	}
	if (!glIsEnabled(GL_DEBUG_OUTPUT))
	{
		LOGI << "Failed to enable GL_DEBUG_OUTPUT";
#ifdef DEVELOPMENT
		fprintf(stderr, "Failed to enable GL_DEBUG_OUTPUT\n");
#endif
	}
	else
	{
		glDebugMessageCallback(handleOpenglError, nullptr);
	}

	// This is supposed to enable vsync, but it gives an error now (because of
	// Daan's OpenGL version?).
	// TODO fix this or find another way to enable vsync
	SDL_ClearError();
	if (SDL_GL_SetSwapInterval(1))
	{
		LOGW << "SDL_GL_SetSwapInterval failed: " << SDL_GetError();
		fprintf(stderr, "SDL_GL_SetSwapInterval failed: %s\n", SDL_GetError());
	}

	// Enable various OpenGL capabilities we will use throughout the code.
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_1D);
	glEnable(GL_TEXTURE_2D);

	// Set a blend function so OpenGL knows what to do with alpha values.
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Set the projection matrix to sensible values. Somehow this is extremely
	// important and we get only four colors on screen if we do not set this.
	glOrtho(0, width(), height(), 0, 0, 1);

	// Initialize SDL_IMG for loading PNGs.
	SDL_ClearError();
	if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG)
	{
		LOGE << "IMG_Init failed: " << IMG_GetError();
		fprintf(stderr, "IMG_Init failed: %s\n", IMG_GetError());
	}

	// Initialize SDL_TTF for loading fonts and displaying text.
	SDL_ClearError();
	if (TTF_Init())
	{
		LOGE << "TTF_Init failed: " << TTF_GetError();
		fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
	}

	// Load the default font.
	{
		std::string root = _settings.resourceRoot.value("");
		if (!root.empty() && root.back() != '/' && root.back() != '\\')
		{
			root += "/";
		}
		std::string fname = _settings.fontFilename.value(
			root + "fonts/GNU_Unifont/unifont.ttf");
		_installedFontFilenames.push_back(fname);
	}

	// Load the shaders.
	for (size_t i = 0; i < SHADER_SIZE; i++)
	{
		loadShader((Shader) i);
	}
	_shaderindexSprite = (size_t) Shader::STANDARD;
	_shaderindexPicture = (size_t) Shader::PICTURE;
	_shaderindexText = (size_t) Shader::TEXT;

	// Initialize ImGui.
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	{
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
	}
	// TODO keyboard controls
	// TODO gamepad controls
	// TODO style
}

void Graphics::loadShader(Shader shader)
{
	const char* shaderName = getShaderName(shader);
	const char* shaderSource = getShaderSource(shader);

	// Some helper variables to determine whether the shader is compiled and
	// linked correctly.
	GLint compiled = GL_FALSE;
	GLint linked = GL_FALSE;

	// Compile our shader program as a fragment shader (aka pixel shader). We
	// only use this and not vertex shader at the moment, because we only want
	// to transform colors and not the vertices themselves.
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &shaderSource, nullptr);
	glCompileShader(fragShader);
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		GLint length = 0;
		glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &length);
		std::vector<GLchar> error(length);
		glGetShaderInfoLog(fragShader, length, &length, &error[0]);
		glDeleteShader(fragShader);
		LOGF << "Failed to compile shader " << shaderName << ": "
			<< std::string(error.begin(), error.end());
		throw std::runtime_error("Failed to compile shader " + std::string(shaderName) + ":\n"
			+ std::string(error.begin(), error.end()));
	}

	// Link the shader.
	GLuint program = glCreateProgram();
	glAttachShader(program, fragShader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		LOGF << "Failed to link shader " << shaderName;
		throw std::runtime_error("Failed to link shader" + std::string(shaderName));
	}

	// Add the shader to the list.
	_shaders.push_back(shader);
	_shaderPrograms.emplace_back(program);

	// Flag the fragment shader for deletion; it will not actually be deleted
	// until the program is deleted
	glDeleteShader(fragShader);
}

void Graphics::clear()
{
	// Clear the OpenGL buffer.
	glClear(GL_COLOR_BUFFER_BIT);
}

void Graphics::flip()
{
	// Swap the buffer with the window buffer.
	SDL_GL_SwapWindow(_window);
}

void Graphics::finish()
{
	if (_finish) glFinish();
}

void Graphics::prepare()
{
	_renderstarttime = SDL_GetTicks();
}

void Graphics::update()
{
	unsigned int endtime = SDL_GetTicks();
	_rendertime[_rendertimeoffset] = endtime - _renderstarttime;
	_rendertimeoffset = (_rendertimeoffset + 1) % 10;

	/* IMGUI */
	static bool show = false;
	bool wasshown = show;

	if (Input::get()->isDebugKeyHeld())
	{
		if (ImGui::Begin("Windows", nullptr,
			ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
		{
			ImGui::Checkbox("Shader", &show);
		}
		ImGui::End();
	}

	if (show)
	{
		if (!wasshown) ImGui::SetNextWindowCollapsed(false);

		if (ImGui::Begin("Shader", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			int index = _shaderindexSprite;
			for (size_t i = 0; i < _shaders.size(); i++)
			{
				if (_shaders[i] == Shader::PICTURE) continue;
				if (_shaders[i] == Shader::TEXT) continue;
				ImGui::RadioButton(getShaderName(_shaders[i]), &index, i);
			}
			_shaderindexSprite = index;

			ImGui::Separator();
			ImVec2 halfsize(200, 40);
			float average = 0;
			for (int i = 0; i < 10; i++)
			{
				average += _rendertime[i] * 0.1f;
			}
			std::string averagestring = std::to_string(average);
			ImGui::PlotLines("Render Time:", _rendertime, 10,
					0, averagestring.c_str(), 0.0f, 16.0f, halfsize);
			ImGui::Checkbox("Finish rendering before next frame", &_finish);

			GLint dedicated;
			GLint total;
			GLint current;
			GLint evicted;
			GLint ecount;
			glGetIntegerv(0x9047, &dedicated);
			glGetIntegerv(0x9048, &total);
			glGetIntegerv(0x9049, &current);
			glGetIntegerv(0x904A, &ecount);
			glGetIntegerv(0x904B, &evicted);
			GLint usage = dedicated - current;
			ImGui::Separator();
			ImGui::Value("Current Memory Usage", usage);
			ImGui::Value("Dedicated Memory", dedicated);
			ImGui::Value("Total Available Memory", total);
			ImGui::Value("Current Available Memory", current);
			ImGui::Value("Evicted Memory", evicted);
			ImGui::Value("Eviction Count", ecount);
			ImGui::Separator();
			ImGui::Checkbox("Debug UI", &_debugUI);
		}
		ImGui::End();
	}
}

void Graphics::raiseWindow()
{
	SDL_RaiseWindow(_window);
}
