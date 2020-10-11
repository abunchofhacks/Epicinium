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
#include "animation.hpp"
#include "source.hpp"

#include "loop.hpp"


Animation::Animation(std::shared_ptr<AnimationGroup> group,
	const std::function<void(float)>& callback, float duration, float delay) :
	_group(group),
	_callback(callback),
	_duration(duration),
	_delay(delay),
	_progress(0)
{}

void Animation::update()
{
	if (_progress >= 1) return;

	float dt = Loop::delta() * Loop::tempo();

	if (_delay > 0)
	{
		_delay -= dt;
		if (_delay >= 0) return;
		else dt = -_delay;
	}

	if (_duration <= 0 || std::isnan(_duration) || std::isinf(_duration))
	{
		_progress = 1;
	}
	else
	{
		_progress += dt / _duration;
		if (_progress > 1) _progress = 1;
	}

	_callback(_progress);
}

bool Animation::finished()
{
	return _progress >= 1;
}
