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

#include <bitset>

#include "animator.hpp"


enum : size_t
{
	TRANSITION_MOVE_X,
	TRANSITION_MOVE_Y,
	TRANSITION_DISPLACE_X,
	TRANSITION_DISPLACE_Y,
	TRANSITION_STUN,
	TRANSITION_LIGHT,
	TRANSITION_SPOTLIGHT,
	TRANSITION_FLASHLIGHT,
	TRANSITION_OBSCURED,
	TRANSITION_SNOW,
	TRANSITION_IMPACT,
	TRANSITION_TEMPERATURE,
	TRANSITION_HUMIDITY,
	TRANSITION_CHAOS,
	TRANSITION_POWER,
	TRANSITION_ENABLED,
	TRANSITION_HOVERED,
	TRANSITION_PRESSED,
	TRANSITION_SIZE
};

class Transitionator : public Animator
{
protected:
	Transitionator();

public:
	virtual ~Transitionator() = default;

private:
	std::bitset<TRANSITION_SIZE> _transitionlocked;

protected:
	std::vector<float> _transitions;

public:
	void lock(size_t index);
	void set(size_t index, float target);
	void transition(std::shared_ptr<AnimationGroup> group, size_t index,
		float target, float duration, float delay);
	void transition(std::shared_ptr<AnimationGroup> group, size_t index,
		float target, float duration);
	void transition(size_t index, float target, float duration, float delay);
	void transition(size_t index, float target, float duration);
};
