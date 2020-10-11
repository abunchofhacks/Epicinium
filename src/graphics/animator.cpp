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
#include "animator.hpp"
#include "source.hpp"

#include "animationgroup.hpp"


void Animator::update()
{
	for (auto animation = _animations.begin(); animation != _animations.end(); /**/)
	{
		animation->update();
		if (animation->finished()) animation = _animations.erase(animation);
		else ++animation;
	}

	if (_personalDelayGroup.lock() == nullptr)
	{
		_personalDelay = 0;
	}
}

void Animator::addAnimation(Animation&& animation)
{
	_animations.emplace_back(animation);
}

void Animator::personalDelay(std::shared_ptr<AnimationGroup> group)
{
	if (!group) return;

	if (auto delaygroup = _personalDelayGroup.lock())
	{
		if (delaygroup.get() == group.get())
		{
			group->delay = std::max(_personalDelay, group->delay);
			return;
		}
	}

	_personalDelay = 0;
}

void Animator::personalDelay(std::shared_ptr<AnimationGroup> group, float delay)
{
	if (auto delaygroup = _personalDelayGroup.lock())
	{
		if (delaygroup.get() == group.get())
		{
			_personalDelay = std::max(_personalDelay, delay);
			return;
		}
	}

	_personalDelay = delay;
	_personalDelayGroup = group;
}

void Animator::pause(std::shared_ptr<AnimationGroup> group, float delay)
{
	addAnimation(Animation(group, [](float /**/) {}, 0, delay));
}

void Animator::reset()
{
	_animations.clear();
}
