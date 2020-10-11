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
#include "mixer.hpp"
#include "source.hpp"

#include <mutex>
#include <future>

#include "libs/SDL2/SDL_audio.h"

#include "settings.hpp"
#include "loop.hpp"


Mixer* Mixer::_installed = nullptr;
static std::mutex _mutex;

void Mixer::callback(void*, uint8_t* buffer, int length)
{
	// Empty buffer?
	if (length <= 0) return;

	// Start with silence.
	memset(buffer, 0, length * sizeof(uint8_t));

	// Mixer muted or not installed? Only silence.
	if (!_installed || _installed->_muted) return;

	{
		std::lock_guard<std::mutex> lock(_mutex);

		// See if any delayed audibles become active.
		for (auto audible = _installed->_delayed.begin();
			audible != _installed->_delayed.end(); /**/)
		{
			if (audible->timeLeft() > _installed->bufferTime())
			{
				audible++;
			}
			else
			{
				std::move(audible, std::next(audible),
					std::back_inserter(_installed->_audibles));
				audible = _installed->_delayed.erase(audible);
			}
		}

		// Apply fades to active audibles.
		while (!_installed->_fades.empty())
		{
			Fade& fade = _installed->_fades.front();
			if (fade.audibleId == 0)
			{
				for (Audible& audible : _installed->_audibles)
				{
					audible.fade(fade.target, fade.duration);
					if (fade.stop) audible.stop();
				}
			}
			else
			{
				for (Audible& audible : _installed->_audibles)
				{
					if (fade.audibleId != audible.id()) continue;
					audible.fade(fade.target, fade.duration);
					if (fade.stop) audible.stop();
					break;
				}
			}
			_installed->_fades.pop();
		}
	}

	// Buffer all active audibles.
	for (auto audible = _installed->_audibles.begin();
		audible != _installed->_audibles.end(); /**/)
	{
		audible->buffer(buffer, length);
		if (audible->finished())
		{
			audible = _installed->_audibles.erase(audible);
		}
		else
		{
			audible++;
		}
	}
}

Mixer::Mixer(Settings& settings) :
	_settings(settings)
{
	_format.freq = 44100;
	_format.format = AUDIO_S16LSB;
	_format.channels = 2;
	_format.samples = settings.audioBufferSize.value();
	_format.callback = callback;
	SDL_AudioSpec temp;
	SDL_ClearError();
	_id = SDL_OpenAudioDevice(nullptr, 0, &_format, &temp, 0);
	if (!_id)
	{
		fprintf(stderr, "SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
		LOGW << "SDL_OpenAudioDevice failed: " << SDL_GetError();
		_muted = true;
		return;
	}
	_format = temp;
	if (_format.channels != 2)
	{
		LOGW << "Could not get two audio channels, muted!";
		_muted = true;
	}
	if (_format.format != AUDIO_S16LSB)
	{
		LOGW << "Could not get 16-bit audio, muted!";
		_muted = true;
	}
	SDL_PauseAudioDevice(_id, 0);
}

Mixer::~Mixer()
{
	std::promise<bool> promiseClose;
	std::future<bool> futureClose = promiseClose.get_future();

	std::thread thread([this](std::promise<bool> promise){

		SDL_CloseAudioDevice(_id);
		promise.set_value(true);

	}, std::move(promiseClose));

	if (futureClose.wait_for(std::chrono::seconds(1))
		!= std::future_status::ready)
	{
		LOGE << "Failed to close audio device in time";
		thread.detach();
	}
	else thread.join();
}

void Mixer::install()
{
	_installed = this;
}

void Mixer::update()
{
	// Delayed audibles have their delays reduced based on animation tempo,
	// so that they start playing right when any corresponding animations start.
	// Because the audio thread does not know about hitstops, we use rawTempo()
	// add manually add the hitstop amounts to the "virtual" delay instead.
	double dt = Loop::delta() * Loop::rawTempo();

	std::lock_guard<std::mutex> lock(_mutex);
	for (Audible& audible : _delayed)
	{
		audible.update(dt);
	}
}

void Mixer::virtualizeHitstops(std::vector<std::pair<float, float>> hitstops)
{
	std::sort(hitstops.begin(), hitstops.end());

	double sum = 0;

	std::lock_guard<std::mutex> lock(_mutex);
	for (auto da : hitstops)
	{
		double virtualDelay = da.first + sum;
		double amount = da.second;
		for (Audible& audible : _delayed)
		{
			if (audible.delay() > virtualDelay)
			{
				audible.delay() += amount;
			}
		}
		sum += amount;
	}
}

uint16_t Mixer::queue(Clip::Type clip, double delay, const Point& point,
	float volume)
{
	std::lock_guard<std::mutex> lock(_mutex);

	_delayed.emplace_back(Clip::get(clip), delay, point, volume);
	return _delayed.back().id();
}

uint16_t Mixer::queue(Clip::Type clip, double delay, bool music, float volume)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_delayed.emplace_back(Clip::get(clip), delay, music, volume);
	return _delayed.back().id();
}

uint16_t Mixer::loop(Clip::Type clip, float volume)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_delayed.emplace_back(Clip::get(clip), volume);
	return _delayed.back().id();
}

void Mixer::loopOST(float volume, float midiVolume) {
	std::lock_guard<std::mutex> lock(_mutex);
	_delayed.emplace_back(Clip::get(Clip::Type::TITLE), volume);
	OSTid = _delayed.back().id();

	_delayed.emplace_back(Clip::get(Clip::Type::TITLE_MIDI), midiVolume);
	midiOSTid = _delayed.back().id();
}

void Mixer::fade(uint16_t audibleId, float target, float duration, bool stop)
{
	std::lock_guard<std::mutex> lock(_mutex);
	if (audibleId == 0)
	{
		// Fade all audibles in _delayed.
		for (Audible& audible : _delayed)
		{
			audible.fade(target, duration);
		}
		// The active audibles in _audibles are faded by callback().
	}
	else
	{
		// If the fade only affects a delayed audible, fade it now.
		for (Audible& audible : _delayed)
		{
			if (audible.id() != audibleId) continue;
			audible.fade(target, duration);
			return;
		}
	}
	_fades.emplace(audibleId, target, duration, stop);
}

void Mixer::stop()
{
	std::lock_guard<std::mutex> lock(_mutex);
	_delayed.clear();
	_fades.emplace(0, 0.0f, 1.0f, true);
}

int Mixer::bufferSize() const
{
	return _format.samples * _format.channels
		* SDL_AUDIO_BITSIZE(_format.format) / 8;
}

double Mixer::bufferTime() const
{
	return _format.samples / double(_format.freq);
}

float Mixer::gameplayVolume() const
{
	return _settings.masterVolume.value()
		*  _settings.gameplayVolume.value();
}

float Mixer::musicVolume() const
{
	return _settings.masterVolume.value()
		*  _settings.musicVolume.value();
}
