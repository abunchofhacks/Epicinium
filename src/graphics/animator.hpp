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

#include "animation.hpp"

struct AnimationGroup;


class Animator
{
protected:
	Animator() = default;
public:
	virtual ~Animator() = default;

private:
	std::vector<Animation> _animations;

	float _personalDelay = 0;
	std::weak_ptr<AnimationGroup> _personalDelayGroup;

public:
	void update();
	void addAnimation(Animation&& animation);
	void personalDelay(std::shared_ptr<AnimationGroup> group);
	void personalDelay(std::shared_ptr<AnimationGroup> group, float delay);
	void pause(std::shared_ptr<AnimationGroup> group, float delay);
	void reset();
};
