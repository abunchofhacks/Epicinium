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
#include "clip.hpp"
#include "source.hpp"

#include <map>
#include <mutex>

#include "mixer.hpp"


std::string Clip::_audiofolder = "audio/";

void Clip::setRoot(const std::string& root)
{
	if (root.empty())
	{
		_audiofolder = "audio/";
	}
	else if (root.back() == '/')
	{
		_audiofolder = root + "audio/";
	}
	else
	{
		_audiofolder = root + "/audio/";
	}
}

static std::map<std::string, Clip> _clips = {};
static std::mutex _clipsMutex;

Clip::Clip(const char* filename) :
	_filename(filename),
	_format(*Mixer::get()->format())
{
	SDL_ClearError();
	if (!SDL_LoadWAV(filename, &_format, &_buffer, &_size))
	{
		LOGE << "Could not load audio clip " << _filename << ": "
			<< SDL_GetError();
		DEBUG_ASSERT(false);
	}

	SDL_AudioCVT cvt;
	if (SDL_BuildAudioCVT(&cvt, _format.format, _format.channels, _format.freq,
		Mixer::get()->format()->format, Mixer::get()->format()->channels,
		Mixer::get()->format()->freq))
	{
		cvt.len = _size;
		cvt.buf = (uint8_t*)(SDL_malloc(cvt.len * cvt.len_mult));
		memcpy(cvt.buf, _buffer, _size);
		SDL_ClearError();
		if (SDL_ConvertAudio(&cvt))
		{
			LOGE << "Could not convert audio clip " << _filename <<
				" to desired format: " << SDL_GetError();
			DEBUG_ASSERT(false);
			return;
		}
		SDL_FreeWAV(_buffer);
		_format = *Mixer::get()->format();
		_size = cvt.len_cvt;
		_buffer = cvt.buf;
	}
}

Clip::~Clip()
{
	SDL_FreeWAV(_buffer);
}

Clip& Clip::load(const char* name)
{
	std::lock_guard<std::mutex> lock(_clipsMutex);
	try
	{
		return _clips.at(name);
	}
	catch (const std::out_of_range&)
	{
		std::string filename = _audiofolder + std::string(name) + ".wav";
		_clips.emplace(name, filename.c_str());
		return _clips.at(name);
	}
}

Clip& Clip::get(Clip::Type type)
{
	switch (type)
	{
		case Clip::Type::NONE      : return load("silence");    break;
		case Clip::Type::TITLE     : return load("title");      break;
		case Clip::Type::TITLE_MIDI     : return load("title_midi");      break;
		case Clip::Type::FOOTSTEP:
		{
			switch (rand() % 3)
			{
				case 0:  return load("footstep1");
				case 1:  return load("footstep2");
				default: return load("footstep3");
			}
		}
		case Clip::Type::UI_CLICK:
		{
			switch(rand() % 3)
			{
				case 0: return load("ui_click1");
				case 1: return load("ui_click2");
				default: return load("ui_click3");
			}
		}
		break;
		case Clip::Type::ENGINE:
		{
			switch (rand() % 3)
			{
				case 0:  return load("engine1");
				case 1:  return load("engine2");
				default: return load("engine3");
			}
		}
		break;
		case Clip::Type::ZEPPELIN:
		{
			switch (rand() % 3)
			{
				case 0:  return load("zeppelin1");
				case 1:  return load("zeppelin2");
				default: return load("zeppelin3");
			}
		}
		break;
		case Clip::Type::EMINATE:
		{
			switch (rand() % 3)
			{
				case 0:  return load("eminate1");
				case 1:	 return load("eminate2");
				default: return load("eminate3");
			}
		}
		break;
		case Clip::Type::GUNSHOT:
		{
			switch (rand() % 3)
			{
				case 0:  return load("gunshot1");
				case 1:  return load("gunshot2");
				default: return load("gunshot3");
			}
		}
		break;
		case Clip::Type::EXPLOSION:
		{
			switch (rand() % 3)
			{
				case 0:  return load("explosion1");
				case 1:  return load("explosion2");
				default: return load("explosion3");
			}
		}
		break;
		case Clip::Type::TREEFELL:
		{
			switch (rand() % 3)
			{
				case 0:  return load("treefell1");
				case 1:  return load("treefell2");
				default: return load("treefell3");
			}
		}
		break;
		case Clip::Type::WHISTLE:
		{
			switch (rand() % 3)
			{
				case 0:  return load("whistle");
				case 1:  return load("whistle2");
				default: return load("whistle3");
			}
		}
		break;
		case Clip::Type::UP             : return load("up1");            break;
		case Clip::Type::DOWN           : return load("down1");          break;
		case Clip::Type::COLLAPSE       : return load("collapse1");      break;
		case Clip::Type::PLACEMENT      : return load("placement1");     break;
		case Clip::Type::SPLASH         : return load("splash1");        break;
		case Clip::Type::COIN           : return load("coin1");          break;
		case Clip::Type::NOCOIN         : return load("nocoin1");        break;
		case Clip::Type::HEART          : return load("heart1");         break;
		case Clip::Type::NOHEART        : return load("noheart1");       break;
		case Clip::Type::PLANNING       : return load("planning1");      break;
		case Clip::Type::TICK           : return load("tick");           break;
		case Clip::Type::TOCK           : return load("tock");           break;
		case Clip::Type::SPRING : return load("season_spring"); break;
		case Clip::Type::ACTION		: return load("action"); break;
		case Clip::Type::SUMMER : return load("season_summer"); break;
		case Clip::Type::AUTUMN : return load("season_autumn"); break;
		case Clip::Type::WINTER : return load("season_winter"); break;
		case Clip::Type::UI_HOVER       : return load("ui_hover");       break;
		case Clip::Type::UI_HOVERHIGHLIGHT       : return load("ui_hoverhighlight");       break;
		case Clip::Type::UI_HOVERBUILDING       : return load("ui_hoverbuilding");       break;
		case Clip::Type::UI_PANNING     : return load("ui_panning");     break;
		case Clip::Type::UI_BLOCKED     : return load("ui_blockedoption");     break;
		case Clip::Type::PLACE     : return load("place");     break;
		case Clip::Type::SOLDIERWALK     : return load("soldierwalk");     break;
		case Clip::Type::PLANMENU     : return load("planmenuselect");     break;
		case Clip::Type::COMMANDSCREEN		: return load("commandscreen");		break;
		case Clip::Type::NIGHTTIME		: return load("nighttime");		break;
		case Clip::Type::CHAOSREPORT		: return load("chaosreport"); break;
		case Clip::Type::WIN		: return load("triumph"); break;
		case Clip::Type::FAIL		: return load("fail"); break;
		case Clip::Type::HEADSUP		: return load("headsup"); break;
		case Clip::Type::GAS		: return load("gas"); break;
		case Clip::Type::CROPS		: return load("crops"); break;
		case Clip::Type::CROPSBURN		: return load("cropsburning"); break;
		case Clip::Type::TRAMPLE		: return load("trample"); break;
		case Clip::Type::FROSTBITE		: return load("frostbite"); break;
		case Clip::Type::FROSTHURT		: return load("frosthurt"); break;
		case Clip::Type::FIRESTORM		: return load("firestorm"); break;
	}
	return load("silence");
}
