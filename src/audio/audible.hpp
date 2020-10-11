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

#include "point.hpp"

class Clip;


class Audible
{
public:
	Audible(const Clip& clip, double delay, const Point& point, float volume);
	Audible(const Clip& clip, double delay, bool music, float volume);
	Audible(const Clip& clip, float volume);

private:
	uint16_t _id;
	const Clip* _clip;
	double _delay;
	double _timer;
	Point _point;

	bool _panned;
	bool _loop;
	bool _music;
	double _lastUpdate;
	uint32_t _progress = 0;
	float _volume;
	float _lastVolume = 1.0f;

	bool _fading = false;
	float _fadeOrigin;
	float _fadeTarget;
	size_t _fadeStart;
	size_t _fadeLength;

public:
	void update(double dt);

	void buffer(uint8_t* buffer, size_t length);

	void fade(float target, float duration);
	void stop() { _loop = false; }

	uint16_t id() const { return _id; }
	bool finished() const;
	double& delay() { return _delay; }
	double timeLeft();
};
