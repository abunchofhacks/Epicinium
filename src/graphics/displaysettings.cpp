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
#include "displaysettings.hpp"
#include "source.hpp"

#include "libs/SDL2/SDL.h"

#include "settings.hpp"


DisplaySettings::DisplaySettings(Settings& settings)
{
	int ndisplays = 0;
	SDL_ClearError();
	if ((ndisplays = SDL_GetNumVideoDisplays()) < 0)
	{
		LOGE << "SDL_GetNumVideoDisplays failed: " << SDL_GetError();
		fprintf(stderr, "SDL_GetNumVideoDisplays failed: %s\n",
			SDL_GetError());
	}

	if (!settings.display.defined()
		|| settings.display.value() > ndisplays)
	{
		settings.defaults().display = 1;
		LOGI << "Using display 1";
	}

	displayBounds.resize(ndisplays);
	//usableBounds.resize(ndisplays);

	for (int d = 0; d < ndisplays; d++)
	{
		SDL_ClearError();
		if (SDL_GetDisplayBounds(d, &displayBounds[d]) != 0)
		{
			LOGE << "SDL_GetDisplayBounds failed: " << SDL_GetError();
			fprintf(stderr, "SDL_GetDisplayBounds failed: %s\n",
				SDL_GetError());
		}

		// SDL_GetDisplayUsableBounds?
	}

	switch (settings.screenmode.value())
	{
		case ScreenMode::WINDOWED:
		case ScreenMode::BORDERLESS:
		{
			int d = std::max(0, settings.display.value() - 1);
			const int w = displayBounds[d].w;
			const int h = displayBounds[d].h;

			if (!settings.width.defined())
			{
				settings.defaults().width = std::min(w,
					std::max(w * 2 / 3, 1280));
				LOGI << "Using native usable width: "
					<< settings.width.value();
			}
			else if (settings.width.value() <= 0)
			{
				settings.width = std::min(w,
					std::max(w * 2 / 3, 1280));
				LOGI << "Using native usable width: "
					<< settings.width.value();
			}
			else if (settings.width.value() > w)
			{
				std::cerr << "Chosen width ("
					<< settings.width.value()
					<< ") is too large; game might be clipped." << std::endl;
				LOGW << "Chosen width ("
					<< settings.width.value()
					<< ") is too large; game might be clipped.";
			}

			if (!settings.height.defined())
			{
				settings.defaults().height = std::min(h,
					std::max(h * 2 / 3, 720));
				LOGI << "Using native usable height: "
					<< settings.height.value();
			}
			else if (settings.height.value() <= 0)
			{
				settings.height = std::min(h,
					std::max(h * 2 / 3, 720));
				LOGI << "Using native usable height: "
					<< settings.height.value();
			}
			else if (settings.height.value() > h)
			{
				std::cerr << "Chosen height ("
					<< settings.height.value()
					<< ") is too large; game might be clipped." << std::endl;
				LOGW << "Chosen height ("
					<< settings.height.value()
					<< ") is too large; game might be clipped.";
			}

			if (!settings.windowX.defined())
			{
				settings.defaults().windowX = -1;
				LOGI << "Using centered window X position.";
			}
			else if (settings.windowX.value()
				+ settings.width.value() > w)
			{
				std::cerr << "Chosen window X position ("
					<< settings.windowX.value()
					<< ") is too large; game might be clipped." << std::endl;
				LOGW << "Chosen window X position ("
					<< settings.windowX.value()
					<< ") is too large; game might be clipped.";
			}

			if (!settings.windowY.defined())
			{
				settings.defaults().windowY = -1;
				LOGI << "Using centered window Y position.";
			}
			else if (settings.windowY.value()
				+ settings.height.value() > h)
			{
				std::cerr << "Chosen window Y position ("
					<< settings.windowY.value()
					<< ") is too large; game might be clipped." << std::endl;
				LOGW << "Chosen window Y position ("
					<< settings.windowY.value()
					<< ") is too large; game might be clipped.";
			}
		}
		break;
		case ScreenMode::FULLSCREEN:
		case ScreenMode::DESKTOP:
		{
			int d = std::max(0, settings.display.value() - 1);
			const int w = displayBounds[d].w;
			const int h = displayBounds[d].h;

			if (!settings.width.defined())
			{
				settings.defaults().width = w;
				LOGI << "Using native display width: "
					<< settings.width.value();
			}
			else if (settings.width.value() <= 0)
			{
				settings.width = w;
				LOGI << "Using native display width: "
					<< settings.width.value();
			}
			else if (settings.width.value() != w)
			{
				std::cerr << "Chosen width ("
					<< settings.width.value()
					<< ") does not match display"
					<< "; game may be stretched or clipped."
					<< std::endl;
				LOGW << "Chosen width ("
					<< settings.width.value()
					<< ") does not match display"
					<< "; game may be stretched or clipped.";
			}

			if (!settings.height.defined())
			{
				settings.defaults().height = h;
				LOGI << "Using native display height: "
					<< settings.height.value();
			}
			else if (settings.height.value() <= 0)
			{
				settings.height = h;
				LOGI << "Using native display height: "
					<< settings.height.value();
			}
			else if (settings.height.value() != h)
			{
				std::cerr << "Chosen height ("
					<< settings.height.value()
					<< ") does not match display"
					<< "; game may be stretched or clipped."
					<< std::endl;
				LOGW << "Chosen height ("
					<< settings.height.value()
					<< ") does not match display"
					<< "; game may be stretched or clipped.";
			}

			if (!settings.windowX.defined())
			{
				settings.defaults().windowX = -1;
				LOGI << "Using centered window X position.";
			}
			else if (settings.windowX.value() + settings.width.value() > w)
			{
				std::cerr << "Chosen window X position ("
					<< settings.windowX.value()
					<< ") is too large; game might be clipped." << std::endl;
				LOGW << "Chosen window X position ("
					<< settings.windowX.value()
					<< ") is too large; game might be clipped.";
			}

			if (!settings.windowY.defined())
			{
				settings.defaults().windowY = -1;
				LOGI << "Using centered window Y position.";
			}
			else if (settings.windowY.value() + settings.height.value() > h)
			{
				std::cerr << "Chosen window Y position ("
					<< settings.windowY.value()
					<< ") is too large; game might be clipped." << std::endl;
				LOGW << "Chosen window Y position ("
					<< settings.windowY.value()
					<< ") is too large; game might be clipped.";
			}
		}
		break;
	}
}

DisplaySettings::~DisplaySettings() = default;
