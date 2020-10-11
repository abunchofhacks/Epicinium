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
#include "transitionator.hpp"
#include "source.hpp"

#include "animationgroup.hpp"


Transitionator::Transitionator()
{
	_transitions.resize(TRANSITION_SIZE, 0);
}

void Transitionator::lock(size_t index)
{
	_transitionlocked[index] = true;
}

void Transitionator::set(size_t index, float target)
{
	if (_transitionlocked[index]) return;
	_transitions[index] = target;
}

void Transitionator::transition(std::shared_ptr<AnimationGroup> group, size_t index,
	float target, float duration, float delay)
{
	bool once = false;
	float origin = 0;

	addAnimation(Animation(group, [=](float progress) mutable {

		if (!once)
		{
			origin = _transitions[index];
			once = true;
		}

		if (_transitionlocked[index]) return;

		_transitions[index] = origin + progress * (target - origin);
	}, duration, delay));
}

void Transitionator::transition(std::shared_ptr<AnimationGroup> group, size_t index,
	float target, float duration)
{
	transition(group, index, target, duration, group ? group->delay : 0);
}

void Transitionator::transition(size_t index,
	float target, float duration, float delay)
{
	transition(nullptr, index, target, duration, delay);
}

void Transitionator::transition(size_t index,
	float target, float duration)
{
	transition(nullptr, index, target, duration, 0);
}
