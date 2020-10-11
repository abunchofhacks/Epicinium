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

#include <queue>

#include "audible.hpp"
#include "clip.hpp"

class Settings;


class Mixer
{
private:
	uint16_t OSTid, midiOSTid;
	static Mixer* _installed;

public:
	static Mixer* get() { return _installed; }
	static void callback(void* userdata, uint8_t* stream, int length);

	uint16_t getOSTid() {return OSTid;}
	uint16_t getMidiOSTid() {return midiOSTid;}

	Mixer(Settings& settings);
	Mixer(const Mixer&) = delete;
	Mixer(Mixer&&) = delete;
	Mixer& operator=(const Mixer&) = delete;
	Mixer& operator=(Mixer&&) = delete;
	~Mixer();

private:
	struct Fade
	{
		uint16_t audibleId;
		float target;
		float duration;
		bool stop;

		Fade(uint16_t a, float t, float d, bool s) :
			audibleId(a),
			target(t),
			duration(d),
			stop(s)
		{}
	};

	SDL_AudioSpec _format;
	SDL_AudioDeviceID _id;
	Settings& _settings;
	bool _muted = false;

	// Only accessed in callback thread
	std::vector<Audible> _audibles;

	// Accessed in both, but after locking mutex
	std::vector<Audible> _delayed;
	std::queue<Fade> _fades;

public:
	void install();

	void update();

	void virtualizeHitstops(std::vector<std::pair<float, float>> hitstops);

	uint16_t queue(Clip::Type clip, double delay, const Point& point,
		float volume = 1.0f);
	uint16_t queue(Clip::Type clip, double delay, bool music = false,
		float volume = 1.0f);
	uint16_t loop(Clip::Type clip, float volume = 1.0f);
	void loopOST(float volume, float midiVolume = 0);
	void fade(uint16_t audibleId, float target, float duration,
		bool stop = false);
	void stop();

	int bufferSize() const;
	double bufferTime() const;
	float gameplayVolume() const;
	float musicVolume() const;
	SDL_AudioSpec* format() { return &_format; }
};
