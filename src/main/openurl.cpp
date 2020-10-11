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
#include "openurl.hpp"
#include "source.hpp"

#ifndef PLATFORMUNIX
#include <windows.h>
#include <shellapi.h>
#endif

#ifdef PLATFORMOSX
#include "libs/SDL2/SDL.h"
#endif

#ifdef PLATFORMOSX
#include "graphics.hpp"
#endif


void System::openURL(const std::string& url)
{
#ifdef PLATFORMOSX
	std::string cmd = "open " + url;
	if (std::system(cmd.c_str()) != 0)
	{
		LOGW << "Opening URL " << url << " failed";
	}
	else
	{
		SDL_Window* window = Graphics::get()->getWindow();
		if (window) SDL_MinimizeWindow(window);
	}
#endif

#ifdef PLATFORMDEBIAN
	std::string cmd = "xdg-open " + url;
	if (std::system(cmd.c_str()) != 0)
	{
		LOGW << "Opening URL " << url << " failed";
	}
#endif

#ifdef PLATFORMWINDOWS
	if (static_cast<int>(reinterpret_cast<uintptr_t>((ShellExecute(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL)))) <= 32)
	{
		LOGW << "Opening URL " << url << " failed";
	}
#endif
}
