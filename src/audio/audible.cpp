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
#include "audible.hpp"
#include "source.hpp"

#include <cmath>

#include "libs/SDL2/SDL.h"

#include "mixer.hpp"
#include "camera.hpp"


static uint16_t counter = 1;


Audible::Audible(const Clip& clip, double delay, const Point& point,
	float volume) :
	_id(counter++),
	_clip(&clip),
	_delay(delay),
	_timer(delay),
	_point(point),
	_panned(true),
	_loop(false),
	_music(false),
	_lastUpdate(SDL_GetTicks() / 1000.0),
	_volume(volume)
{}

Audible::Audible(const Clip& clip, double delay, bool music, float volume) :
	_id(counter++),
	_clip(&clip),
	_delay(delay),
	_timer(delay),
	_point(Point()),
	_panned(false),
	_loop(false),
	_music(music),
	_lastUpdate(SDL_GetTicks() / 1000.0),
	_volume(volume)
{}

Audible::Audible(const Clip& clip, float volume) :
	_id(counter++),
	_clip(&clip),
	_delay(0),
	_timer(0),
	_point(Point()),
	_panned(false),
	_loop(true),
	_music(true),
	_lastUpdate(SDL_GetTicks() / 1000.0),
	_volume(volume)
{}

// update() is called from Mixer::update() <- Engine::doFrame() <- EngineLoop.
void Audible::update(double dt)
{
	if (_timer <= 0) return;

	// Because animation tempo, Loop::rawTempo(), is only updated once
	// per frame, it applies retroactively to the timespan since the last frame,
	// i.e. Loop::delta(). We cannot reliably simulate this in callback(),
	// but fortunately it doesn't really matter what trickery we do while the
	// audible is delayed, given "out of ear, out of mind". The only thing
	// we need to ensure is that if the audible *should* start say 5 ms after
	// frame X, and frame X+1 hasn't started by that point, that it starts.
	// Therefore we deploy a dead-man switch: during frame X we set a timer to
	// 5 ms, and after 5 ms this timer runs out and we start playing the sound.
	double time = SDL_GetTicks() / 1000.0;
	_delay -= dt;
	_timer = _delay;
	_lastUpdate = time;
}

// timeLeft() is called from callback(), which is asynchronous with EngineLoop.
double Audible::timeLeft()
{
	double time = SDL_GetTicks() / 1000.0;
	_timer -= time - _lastUpdate;
	_lastUpdate = time;
	return _timer;
}

void Audible::buffer(uint8_t* buffer, size_t length)
{
	Mixer* mixer = Mixer::get();

	// calculate bytes per sample
	int bps = SDL_AUDIO_BITSIZE(mixer->format()->format) / 8;

	// where should we start in this buffer?
	size_t bufferOffset = 0;
	if (_timer > 0)
	{
		bufferOffset = int(_timer * mixer->format()->freq + 0.5)
			* mixer->format()->channels * bps;
		_timer = 0;
	}

	while (bufferOffset < size_t(length))
	{
		// calculate the size of this clip still left to play
		uint32_t clippedSize = _clip->size() - _progress;

		// does it not fit in this buffer?
		if (bufferOffset + clippedSize > length)
		{
			// then just clip after buffer is full (to be continued next
			// buffer)
			clippedSize = length - bufferOffset;
		}

		// nothing left to play? skip this segment
		if (clippedSize <= 0) break;

		if (!_fading && _volume == 0)
		{
			_progress += clippedSize;
			break;
		}

		float mixerVolume;
		if (_music) mixerVolume = mixer->musicVolume();
		else mixerVolume = mixer->gameplayVolume();

		if (!_fading && _lastVolume != _volume)
		{
			_lastVolume = _volume;
		}

		if (_panned || _fading)
		{
			// we need to pan the buffer
			// TODO optimize?
			std::vector<uint8_t> processed;
			processed.reserve(clippedSize);
			float left = 0.5f;
			float right = 0.5f;
			if (_panned)
			{
				float p = Camera::get()->convert(_point).xenon
					/ float(Camera::get()->width());
				float q = pi() / 8.0f * (2.0f * p + 1.0f);
				left = std::cos(q) / sqrt2();
				right = std::sin(q) / sqrt2();
			}

			// just copy the buffer, but apply multiplier for left and right
			// all the following assumes two-channel, 16-bit audio
			// TODO rewrite for any format
			for (size_t i = 0; i < clippedSize / 4; i++)
			{
				if (_fading && _progress + 4 * i >= _fadeStart)
				{
					if (_progress + 4 * i < _fadeStart + _fadeLength)
					{
						float fadeProgress = (_progress + 4 * i - _fadeStart)
							/ float(_fadeLength);
						_volume = _fadeOrigin + fadeProgress
							* (_fadeTarget - _fadeOrigin);
					}
					if (_progress + 4 * (i + 1) >= _fadeStart + _fadeLength)
					{
						_fading = false;
						_lastVolume = _volume;
						_volume = _fadeTarget;
					}
				}

				int16_t sample;
				sample = _clip->buffer()[_progress + 4 * i]
					| _clip->buffer()[_progress + 4 * i + 1] << 8;
				sample *= left * _volume;
				processed.emplace_back(sample);
				processed.emplace_back(sample >> 8);

				sample = _clip->buffer()[_progress + 4 * i + 2]
						| _clip->buffer()[_progress + 4 * i + 3] << 8;
				sample *= right * _volume;
				processed.emplace_back(sample);
				processed.emplace_back(sample >> 8);
			}
			// mix the panned buffer, at 1/4 volume because the multipliers have
			// already multiplied by half (on average)
			SDL_MixAudioFormat(&buffer[bufferOffset], &processed[0],
				mixer->format()->format, clippedSize,
				mixerVolume * SDL_MIX_MAXVOLUME / 2 + 0.5f);
		}
		else
		{
			// just mix normally (at 1/8 volume)
			SDL_MixAudioFormat(&buffer[bufferOffset],
				&_clip->buffer()[_progress], mixer->format()->format,
				clippedSize,
				_volume * mixerVolume * SDL_MIX_MAXVOLUME / 4 + 0.5f);
		}

		// keep track of progress
		_progress += clippedSize;

		// update the buffer offset
		bufferOffset += clippedSize;

		if (!_loop) break;

		// if we finished the clip, start over
		else if (finished()) _progress = 0;
	}
}

void Audible::fade(float target, float duration)
{
	if (_volume == target) return;
	Mixer* mixer = Mixer::get();
	int bps = SDL_AUDIO_BITSIZE(mixer->format()->format) / 8;

	_fadeOrigin = _volume;
	_fadeTarget = target;
	_fadeStart = _progress;
	_fadeLength = int(duration * mixer->format()->freq + 0.5)
		* mixer->format()->channels * bps;
	_fading = true;
}

bool Audible::finished() const
{
	return  _progress >= _clip->size();
}
