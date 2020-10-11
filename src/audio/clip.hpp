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
 * Can Ur (canur11@gmail.com)
 */
#pragma once
#include "header.hpp"

#include "libs/SDL2/SDL_audio.h"


class Clip
{
private:
	static std::string _audiofolder;

public:
	static void setRoot(const std::string& root);

	// Do not call this constructor! Should only be called by Clip::load
	Clip(const char* filename);
	~Clip();

private:
	uint8_t* _buffer;
	uint32_t _size;
	std::string _filename;
	SDL_AudioSpec _format;

	static Clip& load(const char* name);

public:
	enum class Type : uint8_t
	{
		NONE = 0,
		TITLE,
		TITLE_MIDI,
		FOOTSTEP,
		ENGINE,
		GUNSHOT,
		EXPLOSION,
		TREEFELL,
		UP,
		DOWN,
		COLLAPSE,
		PLACEMENT,
		SPLASH,
		COIN,
		NOCOIN,
		HEART,
		NOHEART,
		PLANNING,
		TICK,
		TOCK,
		SPRING,
		ACTION,
		SUMMER,
		AUTUMN,
		WINTER,
		UI_HOVER,
		UI_HOVERHIGHLIGHT,
		UI_HOVERBUILDING,
		UI_CLICK,
		UI_BLOCKED,
		UI_PANNING,
		PLACE,
		SOLDIERWALK,
		PLANMENU,
		WHISTLE,
		COMMANDSCREEN,
		NIGHTTIME,
		CHAOSREPORT,
		FAIL,
		WIN,
		FROSTBITE,
		FROSTHURT,
		FIRESTORM,
		HEADSUP,
		CROPS,
		CROPSBURN,
		TRAMPLE,
		ZEPPELIN,
		GAS,
		EMINATE,
	};

	static Clip& get(Type type);

	const uint8_t* buffer() const { return _buffer; }
	uint32_t size() const { return _size; }
	const std::string& filename() const { return _filename; }
	const SDL_AudioSpec& format() const { return _format; }
};
