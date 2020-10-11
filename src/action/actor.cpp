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
#include "actor.hpp"
#include "source.hpp"

#include "sprite.hpp"
#include "animationgroup.hpp"
#include "loop.hpp"


Actor::Actor(const std::string& spritename, const Point& point) :
	_sprite(new Sprite(spritename)),
	_point(point),
	_particlebuffer(50)
{}

Actor::Actor(const std::string& spritename) :
	_sprite(new Sprite(spritename)),
	_point(0, 0),
	_particlebuffer(0)
{}

Actor::~Actor() = default;

void Actor::update()
{
	Transitionator::update();
	_visualizer.update();
	mix();
	_sprite->setObscured(_transitions[TRANSITION_OBSCURED]);
	_sprite->update();
	drawSprite();

	_particlebuffer.update();
}

void Actor::setTag(std::shared_ptr<AnimationGroup> group,
	const std::string& tag, float delay)
{
	addAnimation(Animation(group, [this, tag](float /**/) {

		_sprite->setTag(tag);
	}, 0, delay));
}

void Actor::setTrigger(std::shared_ptr<AnimationGroup> group,
	const std::string& tag, const std::string& passivetag, float delay)
{
	addAnimation(Animation(group, [this, tag, passivetag](float /**/) {

		// The passive tag will start playing once the active tag has ended.
		_sprite->setTag(passivetag);
		_sprite->setTagActive(tag);
	}, 0, delay));

	// Keep the group in scope while the animation plays.
	addAnimation(Animation(group, [](float /**/) {

		// Nothing.
	}, _sprite->getTagDuration(tag), delay));
}

void Actor::setTrigger(std::shared_ptr<AnimationGroup> group,
	const std::string& tag, float delay)
{
	addAnimation(Animation(group, [this, tag](float /**/) {

		_sprite->setTagActive(tag);
	}, 0, delay));

	// Keep the group in scope while the animation plays.
	addAnimation(Animation(group, [](float /**/) {

		// Nothing.
	}, _sprite->getTagDuration(tag), delay));
}

void Actor::setVisible(std::shared_ptr<AnimationGroup> group, bool visible, float delay)
{
	addAnimation(Animation(group, [this, visible](float /**/) {

		_sprite->setVisible(visible);
	}, 0, delay));
}

void Actor::setVisible(std::shared_ptr<AnimationGroup> group, bool visible)
{
	setVisible(group, visible, group ? group->delay : 0);
}

void Actor::setVisible(bool visible)
{
	_sprite->setVisible(visible);
}

void Actor::addHitstop(std::shared_ptr<AnimationGroup> group, float delay,
	float multiplier)
{
	if (!group) return;

	float amount = Loop::hitstopAmount() * multiplier;
	addAnimation(Animation(group, [amount](float){

		Loop::hitstop(amount);
	}, 0, delay));

	group->hitstops.emplace_back(delay, amount);
}
